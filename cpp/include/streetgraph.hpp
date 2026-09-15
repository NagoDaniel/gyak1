// streetgraph.hpp -- loads a road network exported by tools/build_graph.py
//
// Deliberately dependency-free (no JSON library, no Boost): students should be
// able to read this file top to bottom and understand every line of it.
//
// The graph is stored as CSR (compressed sparse row) adjacency, which is the
// layout every real routing engine uses: one contiguous array of arcs plus an
// offset array. It is cache-friendly and it is worth teaching.
#pragma once

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace sg {

constexpr double kEarthRadiusM = 6371008.8;

// Spelled out rather than using M_PI: that macro is a POSIX extension, and MSVC
// only defines it if _USE_MATH_DEFINES was set before <cmath> was first pulled
// in by anything. Naming it here keeps the header compiling everywhere.
constexpr double kPi = 3.14159265358979323846;

// Great-circle distance in metres. This is the admissible-heuristic building
// block for A*: it never overestimates road distance, because roads cannot be
// shorter than the straight line on the sphere.
inline double haversine(double lat1, double lon1, double lat2, double lon2) {
    constexpr double p = kPi / 180.0;
    const double a = 0.5 - std::cos((lat2 - lat1) * p) / 2
                   + std::cos(lat1 * p) * std::cos(lat2 * p)
                     * (1 - std::cos((lon2 - lon1) * p)) / 2;
    return 2 * kEarthRadiusM * std::asin(std::sqrt(a));
}

struct Arc {
    int to;        // destination node
    int edge_id;   // index into the undirected edge list (what the viewer draws)
    double length; // metres
    double time;   // seconds, using the edge's speed limit
};

struct Edge {
    int u, v;
    double length_m;
    int speed_kph;
    bool oneway;   // true => traversable only u -> v
};

class Graph {
public:
    int num_nodes() const { return static_cast<int>(lat_.size()); }
    int num_edges() const { return static_cast<int>(edges_.size()); }
    int num_arcs()  const { return static_cast<int>(arcs_.size()); }

    double lat(int v) const { return lat_[v]; }
    double lon(int v) const { return lon_[v]; }
    const Edge& edge(int e) const { return edges_[e]; }

    // Outgoing arcs of v, as a half-open range into the arc array.
    // Usage:  for (const Arc& a : g.out(u)) { ... }
    struct ArcRange {
        const Arc* b;
        const Arc* e;
        const Arc* begin() const { return b; }
        const Arc* end()   const { return e; }
        size_t size() const { return static_cast<size_t>(e - b); }
    };
    ArcRange out(int v) const {
        return {arcs_.data() + offset_[v], arcs_.data() + offset_[v + 1]};
    }

    double straight_line_m(int a, int b) const {
        return haversine(lat_[a], lon_[a], lat_[b], lon_[b]);
    }

    // Nearest node to a lat/lon, by brute force. Fine for city-sized graphs and
    // it keeps the header free of spatial-index machinery.
    int nearest(double la, double lo) const {
        int best = -1;
        double bestd = 1e300;
        for (int i = 0; i < num_nodes(); ++i) {
            const double d = haversine(la, lo, lat_[i], lon_[i]);
            if (d < bestd) { bestd = d; best = i; }
        }
        return best;
    }

    // Resolve a graph directory that was given relative to the project root,
    // even when the program was started from somewhere else.
    //
    // IDEs run your program with the working directory set to the build folder
    // (CLion uses cmake-build-debug/), so a path like "data/graphs/demo-city"
    // would not resolve and you would get a confusing "cannot open" instead of
    // a route. Rather than make every student fix a Run Configuration, look a
    // few directories up as well.
    static std::string resolve(const std::string& dir) {
        std::string prefix;
        for (int up = 0; up < 4; ++up) {
            const std::string cand = prefix + dir;
            std::ifstream probe(cand + "/nodes.csv");
            if (probe) return cand;
            prefix += "../";
        }
        return dir;   // report the original path in the error message
    }

    static Graph load(const std::string& dir_in) {
        const std::string dir = resolve(dir_in);
        Graph g;
        g.read_nodes(dir + "/nodes.csv");
        g.read_edges(dir + "/edges.csv");
        g.build_csr();
        return g;
    }

private:
    std::vector<double> lat_, lon_;
    std::vector<Edge> edges_;
    std::vector<int> offset_;
    std::vector<Arc> arcs_;

    static std::vector<std::string> split(const std::string& s, char sep) {
        std::vector<std::string> out;
        std::string cur;
        std::istringstream ss(s);
        while (std::getline(ss, cur, sep)) out.push_back(cur);
        return out;
    }

    void read_nodes(const std::string& path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("cannot open " + path);
        std::string line;
        std::getline(in, line); // header
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto f = split(line, ',');           // id,osm_id,lat,lon
            lat_.push_back(std::stod(f[2]));
            lon_.push_back(std::stod(f[3]));
        }
    }

    void read_edges(const std::string& path) {
        std::ifstream in(path);
        if (!in) throw std::runtime_error("cannot open " + path);
        std::string line;
        std::getline(in, line); // header
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto f = split(line, ',');           // id,u,v,length_m,speed_kph,oneway,name
            Edge e;
            e.u = std::stoi(f[1]);
            e.v = std::stoi(f[2]);
            e.length_m = std::stod(f[3]);
            e.speed_kph = std::stoi(f[4]);
            e.oneway = (f[5] == "1");
            edges_.push_back(e);
        }
    }

    // Expand undirected edges into directed arcs and sort them by tail node.
    void build_csr() {
        const int n = num_nodes();
        std::vector<int> deg(n + 1, 0);
        for (const Edge& e : edges_) {
            deg[e.u]++;
            if (!e.oneway) deg[e.v]++;
        }
        offset_.assign(n + 1, 0);
        for (int i = 0; i < n; ++i) offset_[i + 1] = offset_[i] + deg[i];

        arcs_.resize(offset_[n]);
        std::vector<int> cursor(offset_.begin(), offset_.end() - 1);
        for (int id = 0; id < num_edges(); ++id) {
            const Edge& e = edges_[id];
            const double t = e.length_m / (e.speed_kph / 3.6);
            arcs_[cursor[e.u]++] = Arc{e.v, id, e.length_m, t};
            if (!e.oneway) arcs_[cursor[e.v]++] = Arc{e.u, id, e.length_m, t};
        }
    }
};

} // namespace sg
