#!/usr/bin/env python3
"""
Build the C++ target, run the student's search, open the viewer.

Compiles by invoking a C++ compiler directly, so CMake is not required. Pass
--use-cmake to build through CMake instead (needed for MSVC).

    python3 tools/run.py
    python3 tools/run.py --graph targu-mures --source 6354 --target 5871
    python3 tools/run.py --no-serve            # build + run only
"""
import argparse
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from paths import (ROOT, EXE, CPP_INCLUDE, CPP_SRC, find_compiler,
                   student_binary)

WARN = ["-Wall", "-Wextra", "-Wno-unused-parameter"]


def die(msg):
    """One clear line, no traceback -- students read the last line of output."""
    print(f"\nerror: {msg}", file=sys.stderr)
    sys.exit(1)


def run(cmd, **kw):
    print("+", " ".join(str(c) for c in cmd))
    if 'stdout' in kw or 'stderr' in kw:
        subprocess.run(cmd, check=True, cwd=ROOT, **kw)
        return
    try:
        done = subprocess.run(cmd, check=True, cwd=ROOT, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, text=True)
        if done.stdout:
            print(done.stdout, end='')
        if done.stderr:
            print(done.stderr, end='', file=sys.stderr)
    except subprocess.CalledProcessError as e:
        # The student's own compiler/program output is the useful part; print it
        # verbatim before we say anything of our own.
        if getattr(e, 'stdout', None):
            print(e.stdout, end='')
        if getattr(e, 'stderr', None):
            print(e.stderr, end='', file=sys.stderr)
        raise


def needs_rebuild(binary, sources):
    if not os.path.isfile(binary):
        return True
    stamp = os.path.getmtime(binary)
    return any(os.path.getmtime(s) > stamp for s in sources if os.path.isfile(s))


def compile_direct(source, name, build_dir):
    """One compiler invocation. The project has no libraries and no link flags,
    so this is genuinely all CMake was doing for us."""
    cxx = find_compiler()
    if cxx is None:
        die("no C++ compiler found.\n"
            "  If you use CLion, build there instead -- it ships its own compiler.")

    src = os.path.join(CPP_SRC, source)
    out = os.path.join(ROOT, build_dir, name + EXE)
    headers = [os.path.join(CPP_INCLUDE, h)
               for h in ("streetgraph.hpp", "trace.hpp")]
    if not needs_rebuild(out, [src] + headers):
        print(f"  {name} is up to date")
        return out

    os.makedirs(os.path.join(ROOT, build_dir), exist_ok=True)
    try:
        run([cxx, "-O2", "-std=c++17", *WARN, "-I", CPP_INCLUDE, src, "-o", out])
    except subprocess.CalledProcessError:
        die(f"{os.path.basename(src)} did not compile -- see the compiler "
            f"messages above.")
    return out


def compile_cmake(build_dir):
    try:
        run(["cmake", "-S", ".", "-B", build_dir])
        run(["cmake", "--build", build_dir, "--config", "Release"])
    except FileNotFoundError:
        die("cmake not found. Drop --use-cmake to compile directly instead.")
    except subprocess.CalledProcessError:
        die("the CMake build failed -- see the messages above.")
    b = student_binary(build_dir)
    if b is None:
        die(f"CMake finished but no student binary appeared under {build_dir}/")
    return b


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graph", default="demo-city", help="name under data/graphs/")
    ap.add_argument("--source", type=int, default=-1)
    ap.add_argument("--target", type=int, default=-1)
    ap.add_argument("--trace", default="trace.txt")
    ap.add_argument("--build-dir", default="build")
    ap.add_argument("--use-cmake", action="store_true",
                    help="build through CMake instead of calling the compiler "
                         "directly (use this for MSVC)")
    ap.add_argument("--no-serve", action="store_true")
    ap.add_argument("--port", type=int, default=8000)
    a = ap.parse_args()

    graph_dir = os.path.join("data", "graphs", a.graph)
    if not os.path.isdir(os.path.join(ROOT, graph_dir)):
        have = sorted(d for d in os.listdir(os.path.join(ROOT, "data", "graphs"))
                      if os.path.isdir(os.path.join(ROOT, "data", "graphs", d)))
        die(f"no graph called {a.graph!r}.\n"
            f"  Available: {', '.join(have) or '(none)'}")

    student = compile_cmake(a.build_dir) if a.use_cmake \
        else compile_direct("student_template.cpp", "student", a.build_dir)

    args = [student, graph_dir]
    if a.source >= 0 and a.target >= 0:
        args += [str(a.source), str(a.target)]
    args.append(a.trace)
    try:
        run(args)
    except subprocess.CalledProcessError as e:
        die(f"the search exited with status {e.returncode} -- see above.")

    if not a.no_serve:
        run([sys.executable, "tools/serve.py", "--graph", a.graph,
             "--trace", a.trace, "--port", str(a.port)])


if __name__ == "__main__":
    main()
