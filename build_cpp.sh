#!/usr/bin/env bash
#
# Simple CMake wrapper script to build the CMake targets in the project.
# Any arguments are passed directly to cmake --build.

cmake -S . -B build
cmake --build build "$@"
