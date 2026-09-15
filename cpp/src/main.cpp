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