cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# note: the subdirectory structure in the build tree is necessary for
# pdmath_compile_latex to work properly (in terms of where outputs are written)

add_subdirectory(exercises)
add_subdirectory(original)
