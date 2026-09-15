# trafik

Write shortest-path algorithms in C++ against real street networks; a browser
viewer replays every step of your search over the map.

Three graphs are included — a small synthetic demo city, Târgu Mureș, and
Romania's national road network — so there is nothing to download. File
formats are documented in `docs/protocol.md`.

## Start here (CLion)

1. **Open this folder** in CLion — the folder itself, not `cpp/`. CLion
   finds `CMakeLists.txt` at the top and configures the project.
2. Build and run the **student** target, pointed at a graph, e.g.
   `data/graphs/demo-city 5 4100 trace.txt`.
3. To watch the search on the map, run the viewer from a terminal:
   `python3 tools/serve.py`

CLion ships with its own compiler and CMake, so **Python is the only thing
you need to install** ([python.org](https://www.python.org/downloads/) —
tick *Add python.exe to PATH*). Windows has no `python3` command; use
`python` or `py`.

You edit `cpp/src/student_template.cpp`. Breakpoints work — stepping through
your own search one settle at a time is the fastest way to find a bug.

<details>
<summary><b>Prefer the command line?</b></summary>

```bash
python3 tools/run.py     # compiles, runs, opens the viewer
```

You need Python 3.7+ and a C++17 compiler.

```bash
# macOS
xcode-select --install
# Debian / Ubuntu
sudo apt install build-essential python3
```

Or drive CMake directly:

```bash
cmake -S . -B build && cmake --build build
./build/student data/graphs/demo-city 5 4100 trace.txt
python3 tools/serve.py
```

</details>

## Viewing without a server

`viewer/index.html` also works standalone — open it directly in a browser
and drag `nodes.csv`, `edges.csv`, `geom.csv`, `meta.json` and your
`trace.txt` onto it. No Python needed.

## Everyday commands

```bash
python3 tools/run.py                                # demo-city, random endpoints
python3 tools/run.py --graph targu-mures --source 6354 --target 5871
python3 tools/run.py --graph romania-national        # the whole country's main roads
python3 tools/run.py --no-serve                      # build + run only
```

On Windows use `python` and `build\student.exe`. Wherever you start the
program from, it finds the map and writes `trace.txt` beside
`CMakeLists.txt`, so running from an IDE works without configuring a
working directory.

## Your side of it

Fill in the search loop in `cpp/src/student_template.cpp`. Everything else —
loading the map, timing, writing the trace, printing the result — is done.

Three calls, placed at the right moments in your own search loop:

```cpp
trace::scan(a.edge_id);               // about to examine this arc
trace::discover(v, newDist, edgeId);  // v's tentative distance improved
trace::settle(u, dist[u]);            // u popped from the queue and finalised
```

That vocabulary is enough for every label-setting search — BFS, Dijkstra, A*,
bidirectional Dijkstra, ALT all decompose into exactly these three events,
which is why the viewer never needs to know which algorithm it is showing.

`cpp/src/dfs_reference.cpp` is a worked depth-first search, included for
contrast: DFS finds *a* path, usually a bad one, and watching it wander is
the point.

```bash
./build/dfs_reference data/graphs/demo-city 5 4100 dfs.txt
python3 tools/serve.py --trace dfs.txt
```

### An alternate starting point

`cpp/src/own.cpp` is a second template style: it parses the graph CSVs
itself instead of using `streetgraph.hpp`, and writes trace lines directly
instead of calling into `trace.hpp`. `cpp/src/own_dfs.cpp` is the same style
trimmed to just a worked DFS, for comparison. Pick whichever starting point
you prefer — both produce the same kind of trace file.

## Layout

```
CMakeLists.txt                project file -- open this folder in CLion
cpp/include/streetgraph.hpp   graph loader, CSR adjacency, haversine
cpp/include/trace.hpp         the three calls you add to your code
cpp/src/student_template.cpp  the file you edit
cpp/src/own.cpp               alternate template, no streetgraph.hpp
cpp/src/own_dfs.cpp           alternate template, worked DFS only
cpp/src/dfs_reference.cpp     worked DFS, for contrast
viewer/index.html             single-file step-by-step viewer
tools/run.py                  compile + run + serve, one command
tools/paths.py                finds the binary, wherever your IDE put it
tools/serve.py                static server for the viewer
data/graphs/demo-city         small synthetic city
data/graphs/targu-mures       9,192 nodes / 11,089 edges
data/graphs/romania-national  55,054 nodes / 61,583 edges -- main roads only
```

## Attribution

Map data in `data/graphs/` is © OpenStreetMap contributors, licensed
[ODbL](https://opendatacommons.org/licenses/odbl/). Credit it in anything you
publish from it, including lab reports and screenshots.
