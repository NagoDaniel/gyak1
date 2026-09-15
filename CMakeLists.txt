cmake_minimum_required(VERSION 3.16)
project(trafik CXX)

# This file lives at the repository root so that opening the trafik folder in
# CLion (or any CMake-aware IDE) gives you the code, the tools, the maps and the
# viewer in one project rather than just cpp/.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Put binaries straight in the build directory. Multi-config generators (Visual
# Studio) would otherwise nest them under build/Debug/ or build/Release/, so the
# tools would have to guess where the binary went.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
foreach(cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR})
endforeach()

add_library(streetgraph INTERFACE)
target_include_directories(streetgraph INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cpp/include)

# Reference DFS, for contrast with a label-setting search.
add_executable(dfs_reference cpp/src/dfs_reference.cpp)
target_link_libraries(dfs_reference PRIVATE streetgraph)

# What students actually edit.
add_executable(student cpp/src/student_template.cpp)
target_link_libraries(student PRIVATE streetgraph)

# Library-free alternate template style (parses CSVs directly, no streetgraph.hpp).
add_executable(own cpp/src/own.cpp)
add_executable(own_dfs cpp/src/own_dfs.cpp)

# Warnings on by default: most first-attempt bugs in this lab are uninitialised
# distances or signed/unsigned comparisons, and the compiler can catch both.
foreach(t student dfs_reference own own_dfs)
  if(MSVC)
    target_compile_options(${t} PRIVATE /W4)
  else()
    target_compile_options(${t} PRIVATE -Wall -Wextra -Wno-unused-parameter)
  endif()
endforeach()
