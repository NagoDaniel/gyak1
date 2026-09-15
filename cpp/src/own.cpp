#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <vector>


using namespace std;

struct Edge {
    int to;
    int edge_id;
    double time;
};

vector<bool> visited;
vector<double> dist;
vector<int> via;

// Union-find, so Kruskal can test "would this edge close a cycle" in
// near-constant time instead of walking the tree to check.
struct DSU {
    vector<int> parent, rank_;
    DSU(int n) : parent(n), rank_(n, 0) {
        for (int i = 0; i < n; ++i) parent[i] = i;
    }
    int find(int x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;           // same tree already -- would cycle
        if (rank_[a] < rank_[b]) swap(a, b);
        parent[b] = a;
        if (rank_[a] == rank_[b]) rank_[a]++;
        return true;
    }
};

void dfs(int curr, vector<vector<Edge>>& adj, int target, ostream& trace){
    visited[curr] = true;
    trace<< "settle " << curr << " " << dist[curr] << '\n';
    if(curr == target){
       ;// return;
    }
    for(const Edge& a : adj[curr]){
        trace << "scan " << a.edge_id << '\n';
        if(!visited[a.to]){
            dist[a.to] = dist[curr] + a.time;
            via[a.to] = a.edge_id;
            trace<< "discover " << a.to << " " << dist[a.to] << " " << a.edge_id << '\n';
            dfs(a.to, adj, target, trace);
        }
    }
}

void bfs(int source, vector<vector<Edge>>& adj, ostream& trace){
    queue<int> q;
    visited[source] = true;
    q.push(source);
    while(!q.empty()){
        int curr = q.front(); q.pop();
        trace << "settle " << curr << " " << dist[curr] << '\n';
        for(const Edge& a : adj[curr]){
            trace << "scan " << a.edge_id << '\n';
            if(!visited[a.to]){
                visited[a.to] = true;
                dist[a.to] = dist[curr] + a.time;
                via[a.to] = a.edge_id;
                trace << "discover " << a.to << " " << dist[a.to] << " " << a.edge_id << '\n';
                q.push(a.to);
            }
        }
    }
}

void dijkstra(int source, vector<vector<Edge>>& adj, ostream& trace){
    fill(dist.begin(), dist.end(), numeric_limits<double>::infinity());
    priority_queue<pair<double,int>, vector<pair<double,int>>, greater<>> pq;
    dist[source] = 0.0;
    pq.push({0.0, source});
    while(!pq.empty()){
        auto [d, curr] = pq.top(); pq.pop();
        if(visited[curr]) continue;
        visited[curr] = true;
        trace << "settle " << curr << " " << dist[curr] << '\n';
        for(const Edge& a : adj[curr]){
            trace << "scan " << a.edge_id << '\n';
            if(!visited[a.to] && dist[curr] + a.time < dist[a.to]){
                dist[a.to] = dist[curr] + a.time;
                via[a.to] = a.edge_id;
                trace << "discover " << a.to << " " << dist[a.to] << " " << a.edge_id << '\n';
                pq.push({dist[a.to], a.to});
            }
        }
    }
}

vector<string> split(const string& line){
    vector<string> out;
    stringstream ss(line);
    string field;
    while (getline(ss, field, ',')) out.push_back(field);
    return out;
}

// Node ids are dense and zero-based, so the row count of nodes.csv IS the
// size every per-node array here needs. Counting edges.csv rows instead gives
// a number ~2x too big, which silently oversizes the DSU and makes the
// "stop after n-1 edges" test never fire.
int count_nodes(const string& dir){
    ifstream f(dir + "/nodes.csv");
    if (!f) { cerr << "cannot open " << dir << "/nodes.csv\n"; exit(1); }
    string line;
    getline(f, line);              // header
    int n = 0;
    while (getline(f, line)) if (!line.empty()) ++n;
    return n;
}

void load_edges(const string dir, vector<vector<Edge>>& adj, vector<pair<int, int>>& edge_endpoints,
                 vector<double>& edge_weight){
    ifstream f(dir + "/edges.csv");
    if (!f) { cerr << "cannot open " << dir << "/edges.csv\n"; exit(1); }
    string line;
    getline(f, line);              // header: id,u,v,length_m,speed_kph,oneway,name
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> f6 = split(line);
        const int id = stoi(f6[0]);
        const int u = stoi(f6[1]);
        const int v = stoi(f6[2]);
        const double length_m = stod(f6[3]);
        const double speed_kph = stod(f6[4]);
        const bool oneway = f6[5] == "1";
        const double time = length_m / (speed_kph * 1000.0 / 3600.0);
        if(u >= adj.size() || v >= adj.size()){
            adj.resize(max(u, v) + 1);
        }
        adj[u].push_back({v, id, time});
        if(id >= edge_endpoints.size()){
            edge_endpoints.resize(id + 1);
            edge_weight.resize(id + 1);
        }
        edge_endpoints[id] = {u, v};
        edge_weight[id] = time;   // MST weight -- swap for length_m for a "shortest total road" tree
        if (!oneway) {
            adj[v].push_back({u, id, time});
        }
    }
}

// Kruskal's MST. Unlike dfs()/a real shortest-path search, there is no single
// source or target -- the tree spans every node -- so "settle" here just
// means "this node has joined the tree", not "shortest distance finalised".
// The viewer doesn't know the difference: it paints whatever gets settled
// blue, so a Kruskal trace still shows the tree growing edge by edge.
void kruskal(int n, vector<pair<int,int>>& edge_endpoints, vector<double>& edge_weight, ostream& trace){
    int m = edge_endpoints.size();
    vector<int> order(m);
    for (int i = 0; i < m; ++i) order[i] = i;
    sort(order.begin(), order.end(),
         [&](int a, int b) { return edge_weight[a] < edge_weight[b]; });

    DSU dsu(n);
    vector<int> tree_edges;
    double total = 0.0;
    int found = 0;

    for (int id : order) {
        // Every edge Kruskal considers, in weight order. Rejected ones stay
        // grey, which is what makes the sorted sweep visible.
        trace << "scan " << id << '\n';
        auto [u, v] = edge_endpoints[id];
        if (!dsu.unite(u, v)) continue;     // would close a cycle -- reject

        total += edge_weight[id];
        tree_edges.push_back(id);

        // The viewer keeps ONE edge per node (treeEdge: node -> edge, set by
        // discover, painted blue by settle). So an accepted edge is only ever
        // drawn if it claims a node -- and ~30% of a Kruskal tree joins two
        // nodes that are BOTH already in it, emitting no discover at all and
        // vanishing from the picture. Claiming u unconditionally fixes that.
        //
        // Re-claiming a node is safe: the viewer only un-paints a superseded
        // edge if it has not settled yet, so u's previous tree edge stays blue.
        trace << "discover " << u << " " << total << " " << id << '\n';
        trace << "settle " << u << " " << total << '\n';

        if (++found == n - 1) break;        // a spanning tree has n-1 edges
    }

    // No "path" line: it is drawn every frame with a shadow blur, which is
    // fine for a route of tens of edges and hangs the viewer at n-1 of them.
    // The tree is already fully painted by the settles above.
}

int main(int argc, char** argv) {
    if (argc != 2){
        std::cerr << "usage: bfs_cout_demo <dir>\n";
        return 1;
    }
    int source = 3;
    int target = 1281;
    
    string dir = argv[1];
    vector<vector<Edge>> adj;
    vector<pair<int, int>> edge_endpoints;
    vector<double> edge_weight;

    const int n = count_nodes(dir);
    load_edges(dir, adj, edge_endpoints, edge_weight);
    visited.assign(n, false);
    dist.assign(n, 0.0);
    via.assign(n, -1);

    ofstream trace("trace.txt");
    trace << "meta graph " << dir << '\n';
    trace << "meta source " << source << '\n';
    trace << "meta target " << target << '\n';

   

    //kruskal(n, edge_endpoints, edge_weight, trace);

    //bfs(source, adj, trace);

   // dijkstra(source, adj, trace);

   // dfs single-path mode, kept for comparison:
    // trace << "discover " << source << " " << 0.0 << " " << -1 << '\n';
    dfs(source, adj, target, trace);
    // vector<int> path;
    // int cur = target;
    // while( cur != source ){
    //     path.push_back(via[cur]);
    //     const auto& e = edge_endpoints[via[cur]];
    //     cur = (e.first == cur) ? e.second : e.first;
    // }
    reverse(path.begin(), path.end());
    trace << "path " << dist[target];
    for(int e : path) trace << " " << e;
    trace << '\n';

    // trace.close();
}