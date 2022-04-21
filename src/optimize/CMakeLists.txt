cmake_minimum_required(VERSION 3.16)

# note: what if we add source files?
add_library(optimize INTERFACE)
target_include_directories(optimize INTERFACE ${PDMATH_INCLUDE})
