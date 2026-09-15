// Depth-first search, done the way that actually looks like DFS.
//
//   ./dfs_reference <graph_dir> <source> <target> [trace_out]
//
// Read this next to a Dijkstra run. DFS is NOT a shortest-path algorithm: it
// finds *a* path, usually a badly wrong one, and the point of visualising it is
// to see exactly how it wanders. The three differences from Dijkstra that
// matter are marked (1), (2) and (3) below.
#include "streetgraph.hpp"
#include "trace.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

using sg::Arc;
using sg::Graph;

static Graph g;
static int target_node;
static std::vector<char> visited;
static std::vector<double> cost;   // cost along the DFS tree path, NOT a shortest distance
static std::vector<int> via;
static bool found = false;
static long long pops = 0, pushes = 0, scans = 0;

static void dfs(int u) {
    // (1) Mark on ENTRY, not on exit.
    //
    // This is what makes it a depth-first search at all: it is how a node is
    // stopped from being entered a second time, and it is what bounds the
    // recursion on a graph with cycles. Marking on the way back out means a
    // node stays "unvisited" for the whole time its own subtree is being
    // explored, so a cycle can walk straight back into it.
    visited[u] = 1;

    // (2) Report the node as settled on ENTRY too (pre-order).
    //
    // The viewer paints settled nodes blue, so emitting this on the way back up
    // means nothing turns blue until the recursion unwinds -- the animation
    // shows a spreading orange flood and then a sudden blue backwash, which is
    // the opposite of the deep single-file probe DFS actually performs.
    trace::settle(u, cost[u]);
    ++pops;

    if (u == target_node) {           // stop at the first path found
        found = true;
        return;
    }

    for (const Arc& a : g.out(u)) {
        trace::scan(a.edge_id);
        ++scans;

        // (3) Descend into every unvisited neighbour, full stop.
        //
        // Not "descend if this route is cheaper". A distance test here turns
        // the traversal into a label-correcting relaxation whose shape is
        // driven by edge weights rather than by graph structure, which is why
        // it stops looking like DFS.
        if (!visited[a.to]) {
            cost[a.to] = cost[u] + a.time;
            via[a.to] = a.edge_id;
            ++pushes;
            trace::discover(a.to, cost[a.to], a.edge_id);
            dfs(a.to);
            if (found) return;        // unwind without exploring anything else
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "usage: %s <graph_dir> <source> <target> [trace_out]\n", argv[0]);
        return 2;
    }
    const std::string dir = argv[1];
    const int source = std::atoi(argv[2]);
    target_node = std::atoi(argv[3]);
    if (argc > 4) trace::open(argv[4]);

    g = Graph::load(dir);
    trace::begin(dir, source, target_node);
    trace::note("algorithm", "dfs");

    const int n = g.num_nodes();
    visited.assign(n, 0);
    cost.assign(n, 0.0);
    via.assign(n, -1);

    const auto t0 = std::chrono::steady_clock::now();
    trace::discover(source, 0.0, -1);
    dfs(source);
    const auto t1 = std::chrono::steady_clock::now();
    const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    trace::note("pops", static_cast<double>(pops));
    trace::note("pushes", static_cast<double>(pushes));
    trace::note("edge_scans", static_cast<double>(scans));
    trace::note("elapsed_ms", ms);

    if (!found) {
        std::printf("no path from %d to %d\n", source, target_node);
        return 1;
    }

    std::vector<int> path;
    int cur = target_node;
    while (cur != source) {
        path.push_back(via[cur]);
        const sg::Edge& e = g.edge(via[cur]);
        cur = (e.u == cur) ? e.v : e.u;
    }
    std::reverse(path.begin(), path.end());

    // cost[] is the cost of the tree path, which is a genuine cost for the
    // route DFS returned -- it is simply not the minimum.
    trace::path(path, cost[target_node]);
    std::printf("dfs: cost=%.1f s over %zu edges | visited %lld | scans %lld | %.1f ms\n",
                cost[target_node], path.size(), pops, scans, ms);
    return 0;
}
