# The trace protocol

A trace is a plain text file. One event per line, fields separated by single
spaces. Order matters: the viewer replays the file top to bottom, and that
replay *is* the animation. Nothing is inferred or re-simulated, so what students
see is exactly what their code did.

The format is deliberately boring. It is greppable, diffable, readable in a text
editor, and writable from any language — so a student who wants to prototype in
Python before writing the C++ can, and the same viewer will play it back.

## Graph file formats

A graph lives in one directory with three CSV files and a metadata file.
Node and edge ids are dense and zero-based, which is what makes them usable as
array indices in C++ without a hash map.

`nodes.csv` — one row per intersection.

```
id,osm_id,lat,lon
0,251034127,46.5401230,24.5583910
```

`edges.csv` — one row per street segment between two intersections. Edges are
stored **undirected** with a `oneway` flag; the loader expands them into
directed arcs. `oneway=1` means traversable only in the `u → v` direction.

```
id,u,v,length_m,speed_kph,oneway,name
0,0,1,132.44,50,0,Strada Bolyai
```

`geom.csv` — the shape points between the two endpoints, used only for drawing.
Real streets bend; the routing graph does not care, but the picture does.

```
id,coords
0,46.540123:24.558391 46.540455:24.559102 46.541001:24.559880
```

`meta.json` — bounding box, counts, and the attribution string the viewer
displays.

## Events

| Event | Fields | Meaning |
|---|---|---|
| `meta` | `key value` | `graph <dir>`, `source <node>`, `target <node>` |
| `phase` | `name` | label for multi-stage searches, e.g. `forward` / `backward` |
| `scan` | `edge_id` | an arc is about to be examined |
| `discover` | `node dist via_edge` | node's tentative distance improved to `dist`, reached via `via_edge` (`-1` at the source) |
| `settle` | `node dist` | node popped from the queue and finalised |
| `path` | `cost e1 e2 ...` | the answer, as edge ids from source to target |
| `note` | `key value` | a counter or label for the stats panel |

Everything else is ignored, so students can add their own lines for debugging
without breaking playback.

## What the viewer does with each event

`scan` paints the edge dim yellow — the "I looked at this" trail. `discover`
paints `via_edge` orange and records it as the node's tree edge. `settle` repaints
that tree edge blue. Because a node is discovered before it is settled, and
settled at most once, the picture only ever gains ink; that is what keeps
playback fast on large graphs.

## Why these five events and not more

They are the complete vocabulary of a label-setting search. BFS, Dijkstra, A*,
bidirectional Dijkstra and ALT all decompose into exactly scan / discover /
settle. Adding events per algorithm would push algorithm-specific knowledge into
the viewer, and then the viewer would need changing every time a new lab is
added. It does not.

Bellman-Ford and other label-correcting algorithms settle nodes more than once.
They still replay correctly — the animation shows labels being revised, which is
the interesting part — but `validate_trace.py`'s "no node settled twice" check
does not apply to them and should be skipped.

## Size and performance

One `settle` plus one `scan` per arc is the natural volume, so the trace has
roughly `2 × arcs` lines. Rough figures, measured on the demo graph and scaled:

| Graph | Nodes | Trace lines | File size |
|---|---|---|---|
| demo-city | 4.2 k | 24 k | 0.5 MB |
| Târgu Mureș | ~12 k | ~70 k | 1.5 MB |
| Cluj-Napoca | ~45 k | ~260 k | 6 MB |
| Bucharest | ~120 k | ~700 k | 16 MB |

Text is not the bottleneck: the buffered `fprintf` in `trace.hpp` costs a few
hundred nanoseconds per line, and parsing 700 k lines in the browser takes well
under a second. If a trace ever does get unwieldy, the fix is to drop `scan`
events — they are the majority of the volume and the least informative — rather
than to change the format.

## Guarantees a well-behaved trace satisfies

`validate_trace.py` checks these, and they are worth stating to students up
front because most bugs violate one of them:

1. Every node is settled at most once.
2. A node is discovered before it is settled.
3. Every settled label equals the true shortest distance to that node.
4. The reported path is a connected walk from source to target that respects
   oneway restrictions, and its recomputed cost matches the reported cost.
5. For Dijkstra and BFS, nodes are settled in non-decreasing distance order.
   This does **not** hold for A*, which settles in non-decreasing `f = g + h`
   order — the trace records `g`, so the check is skipped for A*.
