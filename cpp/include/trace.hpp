// trace.hpp -- the only thing a student adds to their algorithm.
//
// Each call appends one line to the trace file. The viewer replays those lines
// in order, so the animation is a faithful recording of what the student's code
// actually did -- not a re-implementation of it.
//
// The trace goes to its own file (default trace.txt, override with the
// TRACE_OUT environment variable or trace::open("...")), so std::cout stays
// free for the student's own debugging output.
//
// Protocol (one event per line, space separated) is documented in docs/protocol.md.
#pragma once

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

namespace trace {

namespace detail {
inline std::FILE*& stream() {
    static std::FILE* f = nullptr;
    return f;
}

// Write the trace at the project root, not next to the executable.
//
// IDEs start your program inside the build folder, so a plain "trace.txt" would
// land in cmake-build-debug/ where tools/serve.py does not look for it. An
// absolute path is used as given; a relative one is placed beside the root
// CMakeLists.txt, found by looking a few directories up.
inline std::string resolve_out(const std::string& path) {
    const bool absolute = !path.empty() &&
        (path[0] == '/' || path[0] == '\\' ||
         (path.size() > 1 && path[1] == ':'));   // C:\... on Windows
    if (absolute) return path;
    std::string prefix;
    for (int up = 0; up < 4; ++up) {
        if (std::ifstream(prefix + "CMakeLists.txt")) return prefix + path;
        prefix += "../";
    }
    return path;
}
inline std::FILE* out() {
    std::FILE*& f = stream();
    if (!f) {
        const char* env = std::getenv("TRACE_OUT");
        f = std::fopen(resolve_out(env ? env : "trace.txt").c_str(), "w");
        if (!f) { std::perror("trace: cannot open output"); std::exit(1); }
        // A big buffer matters: a city-sized Dijkstra emits hundreds of
        // thousands of lines and unbuffered writes dominate the runtime.
        static std::vector<char> buf(1 << 20);
        std::setvbuf(f, buf.data(), _IOFBF, buf.size());
        std::atexit([] { if (stream()) std::fflush(stream()); });
    }
    return f;
}
} // namespace detail

inline void open(const std::string& path) {
    detail::stream() = std::fopen(detail::resolve_out(path).c_str(), "w");
}

// --- setup ---------------------------------------------------------------

// Which graph, and which endpoints. Call once, first.
inline void begin(const std::string& graph_dir, int source, int target) {
    std::fprintf(detail::out(), "meta graph %s\nmeta source %d\nmeta target %d\n",
                 graph_dir.c_str(), source, target);
}

// Optional label for multi-stage searches, e.g. "forward" / "backward" in a
// bidirectional Dijkstra. The viewer tints each phase differently.
inline void phase(const std::string& name) {
    std::fprintf(detail::out(), "phase %s\n", name.c_str());
}

// --- the three events that make up any label-setting search ---------------

// About to look at this edge. Drawn as a brief flash.
inline void scan(int edge_id) {
    std::fprintf(detail::out(), "scan %d\n", edge_id);
}

// Node's tentative distance improved to `dist`, reached via `via_edge`
// (use -1 for the source). Drawn as frontier colour, and builds the search tree.
inline void discover(int node, double dist, int via_edge) {
    std::fprintf(detail::out(), "discover %d %.3f %d\n", node, dist, via_edge);
}

// Node popped from the queue and finalised. Drawn as settled colour.
inline void settle(int node, double dist) {
    std::fprintf(detail::out(), "settle %d %.3f\n", node, dist);
}

// --- results -------------------------------------------------------------

// The answer, as the sequence of edge ids from source to target.
inline void path(const std::vector<int>& edge_ids, double total_cost) {
    std::fprintf(detail::out(), "path %.3f", total_cost);
    for (int e : edge_ids) std::fprintf(detail::out(), " %d", e);
    std::fputc('\n', detail::out());
}

// Free-form counters shown in the viewer's stats panel: heap pushes, pops,
// decrease-keys, elapsed milliseconds. This is how students compare A* to
// Dijkstra quantitatively, not just visually.
inline void note(const std::string& key, double value) {
    std::fprintf(detail::out(), "note %s %.6g\n", key.c_str(), value);
}
inline void note(const std::string& key, const std::string& value) {
    std::fprintf(detail::out(), "note %s %s\n", key.c_str(), value.c_str());
}

} // namespace trace
