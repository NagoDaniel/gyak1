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

add_executable(main cpp/src/main.cpp)

# Warnings on by default: most first-attempt bugs in this lab are uninitialised
# distances or signed/unsigned comparisons, and the compiler can catch both.
if(MSVC)
  target_compile_options(main PRIVATE /W4)
else()
  target_compile_options(main PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()
