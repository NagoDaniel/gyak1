#!/usr/bin/env python3
"""
Serve the repository root so the viewer can fetch graphs and traces.

Browsers block fetch() from file:// URLs, so the viewer needs a server -- but
only a static one. Run this from anywhere:

    python3 tools/serve.py

Then open the URL it prints. Add ?graph=<name>&trace=<path> to load directly,
e.g.  http://localhost:8000/viewer/?graph=demo-city&trace=trace.txt
"""
import argparse
import functools
import http.server
import os
import socketserver
import sys
import webbrowser

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from paths import ROOT


class Handler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Traces change on every run; without this the browser serves a stale one.
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def log_message(self, fmt, *args):
        pass  # keep the terminal readable while students iterate


class Server(socketserver.ThreadingTCPServer):
    # The viewer fetches several files at once (nodes, edges, geom, then a trace
    # that can run to megabytes); serving them one at a time makes a reload
    # visibly slow.
    daemon_threads = True
    # POSIX SO_REUSEADDR only skips TIME_WAIT, but on Windows it lets a second
    # process bind a port that is already in use and steal the listener.
    allow_reuse_address = (os.name != "nt")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=8000)
    ap.add_argument("--graph", default="demo-city")
    ap.add_argument("--trace", default="trace.txt")
    ap.add_argument("--host", default="127.0.0.1",
                    help="0.0.0.0 serves the viewer to your whole network -- "
                         "handy for a projector, off by default")
    ap.add_argument("--no-open", action="store_true")
    a = ap.parse_args()

    os.chdir(ROOT)
    url = f"http://localhost:{a.port}/viewer/?graph={a.graph}&trace={a.trace}"
    with Server((a.host, a.port), functools.partial(Handler, directory=ROOT)) as httpd:
        print(f"serving {ROOT} on {a.host}:{a.port}\n  {url}\nCtrl-C to stop")
        if not a.no_open:
            try:
                webbrowser.open(url)
            except Exception:
                pass
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print()
