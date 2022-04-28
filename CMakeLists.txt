cmake_minimum_required(VERSION 3.16)

project(
    phetdam-math
    VERSION 0.1.0
    DESCRIPTION "Accompanying code for my independent mathematical work."
    HOMEPAGE_URL "https://github.com/phetdam/math"
    LANGUAGES CXX
)

# build debug by default; we set -DCMAKE_BUILD_TYPE=Release for release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# debug and release flags
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -g")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall -O3")

# Boost >=1.71 is a hard dependency
find_package(Boost 1.71 REQUIRED)

# GoogleTest >=1.10 is a [soft] dependency, only enable testing if satisfied
find_package(GTest 1.10)
if(GTest_FOUND AND PACKAGE_VERSION_COMPATIBLE)
    enable_testing()
endif()

# include gets it own variable as we will need that later
set(PDMATH_INCLUDE include)
include_directories(${PDMATH_INCLUDE})

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(test)
