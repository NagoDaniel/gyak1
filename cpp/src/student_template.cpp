// Student template: implement your search here.
//
//   ./student <graph_dir> [<source> <target>] [trace_out]
//
// Fill in bfs() below. Call trace::scan(edge_id) before looking at where an
// arc leads, trace::discover(v, newDist, edgeId) whenever v's tentative
// distance improves, and trace::settle(u, dist[u]) when u is finalised --
// those three calls are all the viewer needs to replay your search.
#include "streetgraph.hpp"
#include "trace.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <limits>
#include <queue>
#include <random>
#include <string>
#include <vector>

using sg::Arc;
using sg::Graph;

static Graph g;
static int target_node;
static std::vector<char> visited;
static std::vector<double> cost;
static std::vector<int> via;
static bool found = false;
static long long pops = 0, pushes = 0, scans = 0;

void bfs(int source, int target) {
    // ============= CODE HERE ==========================


    // ===================================================
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s <graph_dir> [<source> <target>] [trace_out]\n", argv[0]);
        return 2;
    }
    const std::string dir = argv[1];
    g = Graph::load(dir);
    const int n = g.num_nodes();

    int source;
    std::string trace_out;
    if (argc >= 4) {
        source = std::atoi(argv[2]);
        target_node = std::atoi(argv[3]);
        if (argc > 4) trace_out = argv[4];
    } else {
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> pick(0, n - 1);
        source = pick(rng);
        do { target_node = pick(rng); } while (target_node == source);
        std::printf("no source/target given -- picked randomly: %d -> %d\n", source, target_node);
        if (argc == 3) trace_out = argv[2];
    }
    if (!trace_out.empty()) trace::open(trace_out);

    trace::begin(dir, source, target_node);
    trace::note("algorithm", "bfs");

    visited.assign(n, 0);
    cost.assign(n, 0.0);
    via.assign(n, -1);

    const auto t0 = std::chrono::steady_clock::now();
    trace::discover(source, 0.0, -1);
    bfs(source, target_node);
    const auto t1 = std::chrono::steady_clock::now();
    const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    trace::note("pops", static_cast<double>(pops));
    trace::note("pushes", static_cast<double>(pushes));
    trace::note("edge_scans", static_cast<double>(scans));
    trace::note("elapsed_ms", ms);

    if (!found) {
        std::printf("no path from %d to %d\n", source, target_node);
        return 0;
    }

    std::vector<int> path;
    int cur = target_node;
    while (cur != source) {
        path.push_back(via[cur]);
        const sg::Edge& e = g.edge(via[cur]);
        cur = (e.u == cur) ? e.v : e.u;
    }
    std::reverse(path.begin(), path.end());

    trace::path(path, cost[target_node]);
    std::printf("cost=%.1f s over %zu edges | visited %lld | scans %lld | %.1f ms\n",
                cost[target_node], path.size(), pops, scans, ms);
    return 0;
}
