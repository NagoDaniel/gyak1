#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
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

vector<string> split(const string& line){
    vector<string> out;
    stringstream ss(line);
    string field;
    while (getline(ss, field, ',')) out.push_back(field);
    return out;
}

// Node ids are dense and zero-based, so the row count of nodes.csv IS the
// size every per-node array here needs.
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
        edge_weight[id] = time;
        if (!oneway) {
            adj[v].push_back({u, id, time});
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2){
        std::cerr << "usage: own_dfs <dir>\n";
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

    trace << "discover " << source << " " << 0.0 << " " << -1 << '\n';
    dfs(source, adj, target, trace);
}
