#!/usr/bin/env python3
"""Where things live, and which compiler to use.

Shared by run.py and serve.py so they cannot disagree about where the
student binary ended up -- which matters, because CLion and the command line put
it in different places.
"""
import glob
import os
import shutil

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GRAPHS = os.path.join(ROOT, "data", "graphs")
CPP_INCLUDE = os.path.join(ROOT, "cpp", "include")
CPP_SRC = os.path.join(ROOT, "cpp", "src")

EXE = ".exe" if os.name == "nt" else ""

# CMake multi-config generators (Visual Studio) nest binaries per configuration.
# The root CMakeLists pins the output directory to avoid this, but a student may
# still have an older build tree lying around, so keep looking in both places.
_CONFIGS = ("Release", "Debug", "RelWithDebInfo", "MinSizeRel")


def binary_candidates(name, build_dir="build"):
    """Every place `name` could plausibly have been built, as absolute paths.

    CLion does not build into build/. It creates cmake-build-<profile>/, where
    the profile name is user-editable -- cmake-build-debug and
    cmake-build-debug-mingw are both common -- so those are matched by glob.
    """
    exe = name + EXE
    roots = [os.path.join(ROOT, build_dir)]
    roots += sorted(glob.glob(os.path.join(ROOT, "cmake-build-*")))
    out = []
    for r in roots:
        out.append(os.path.join(r, exe))
        out += [os.path.join(r, c, exe) for c in _CONFIGS]
    return out


def find_binary(name, build_dir="build"):
    """Newest build of `name`, or None. Newest wins so that a student who just
    pressed Build in CLion sees that code run, not a stale command-line build."""
    found = [p for p in binary_candidates(name, build_dir) if os.path.isfile(p)]
    return max(found, key=os.path.getmtime) if found else None


def student_binary(build_dir="build"):
    return find_binary("student", build_dir)


def find_compiler():
    """A C++ compiler to invoke directly, or None.

    MSVC's cl is deliberately not looked for: it only works inside a Developer
    Command Prompt, and locating vcvarsall is exactly the job CMake does. Anyone
    on MSVC should use the CMake path (run.py --use-cmake, or CLion).
    """
    env = os.environ.get("CXX")
    if env and shutil.which(env):
        return shutil.which(env)
    for c in ("c++", "g++", "clang++"):
        p = shutil.which(c)
        if p:
            return p
    return None


BUILD_HINT = ("python tools/run.py   (compiles for you -- no CMake needed)\n"
              "  or, with CMake:     cmake -S . -B build && cmake --build build")
