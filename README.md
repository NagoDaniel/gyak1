# trafik

## Fajlok

```
CMakeLists.txt                projekt fajl -- ezt a mappat nyisd meg CLionban
cpp/include/streetgraph.hpp   grafbetolto, CSR adjacencia, haversine
cpp/include/trace.hpp         3 hivas amit a sajat kododba teszel
cpp/src/student_template.cpp  ezt a fajlt szerkeszted (streetgraph.hpp-t hasznalja)
cpp/src/own.cpp               alternativ sablon, CSV-t maga parsolja
cpp/src/own_dfs.cpp           alternativ sablon, csak kesz DFS
cpp/src/dfs_reference.cpp     kesz DFS, osszehasonlitasnak
viewer/index.html             egyfajlos lepesenkenti viewer
tools/run.py                  fordit + fut + kiszolgal, egy paranccsal
tools/paths.py                megtalalja a binarist, barhova is tette az IDE
tools/serve.py                statikus szerver a viewernek
data/graphs/demo-city         kicsi szintetikus varos
data/graphs/targu-mures       9192 csomopont / 11089 el
data/graphs/romania-national  55054 csomopont / 61583 el -- csak fout
docs/protocol.md              trace fajl formatuma
```

## Futtatas

```bash
python3 tools/run.py                                # demo-city, veletlen vegpontok
python3 tools/run.py --graph targu-mures --source 6354 --target 5871
python3 tools/run.py --graph romania-national
python3 tools/run.py --no-serve                      # csak fordit + fut
```

Vagy CMake kozvetlenul:

```bash
cmake -S . -B build && cmake --build build
./build/student data/graphs/demo-city 5 4100 trace.txt
python3 tools/serve.py
```

Viewer szerver nelkul is mukodik: nyisd meg `viewer/index.html`-t, huzd ra
`nodes.csv`, `edges.csv`, `geom.csv`, `meta.json` es `trace.txt` fajlokat.
