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

set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED TRUE)

# debug and release flags, handle MSVC case specifically
if(MSVC)
    add_compile_options(
        /Wall
        /wd4265
        /wd4514
        /wd4625
        /wd4626
        /wd4820
        /wd5026
        /wd5027
        /wd5045
    )
    # MSVC issues C4996 due to use of std::iterator in Boost
    add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    if(CMAKE_BUILD_TYPE STREQUAL Release)
        add_compile_options(/O2)
    else()
        add_compile_options(/Od /DEBUG)
    endif()
else()
    add_compile_options(-Wall -Wextra)
    if(CMAKE_BUILD_TYPE STREQUAL Release)
        add_compile_options(-O3)
    else()
        add_compile_options(-O0 -ggdb)
    endif()
endif()

# macros to define based on release type
if(CMAKE_BUILD_TYPE STREQUAL Release)
    add_compile_definitions(EIGEN_NO_DEBUG)
endif()

# minimum versions for dependencies
set(BOOST_MIN_VERSION 1.71.0)
set(EIGEN_MIN_VERSION 3.4)
set(GTEST_MIN_VERSION 1.10.0)

# EIGEN3_DIR with Eigen3 includes can be set, or if not set on Linux, inferred.
# either pass it using -D or set it as environment variable
if(NOT DEFINED EIGEN3_DIR)
    # read from environment if possible
    if(DEFINED ENV{EIGEN3_DIR})
        set(EIGEN3_DIR $ENV{EIGEN3_DIR})
    # else use default on UNIX, fail on Windows
    elseif(UNIX)
        set(EIGEN3_DIR /usr/local/include/eigen3)
        message(STATUS "EIGEN3_DIR not set, using default ${EIGEN3_DIR}")
    else()
        message(
            FATAL_ERROR "EIGEN3_DIR not set, must be set manually on Windows"
        )
    endif()
endif()

# fix path on Windows
if(WIN32)
    file(TO_CMAKE_PATH ${EIGEN3_DIR} EIGEN3_DIR)
    message(STATUS "EIGEN3_DIR normalized to ${EIGEN3_DIR}")
endif()

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)
find_package(GTest ${GTEST_MIN_VERSION})

# base include for this repo
set(PDMATH_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

include_directories(
    ${PDMATH_INCLUDE}
    # in case Boost and GoogleTest were found using BOOST_ROOT, GTEST_ROOT.
    # the SYSTEM prefix tells compilers to skip warnings not in our control
    SYSTEM ${Boost_INCLUDE_DIR}
    SYSTEM ${GTEST_INCLUDE_DIRS}
    SYSTEM ${EIGEN3_DIR}
)

add_subdirectory(src)
add_subdirectory(examples)

# only build tests and enable testing if compatible GoogleTest is found.
# note: on Windows, GTest_VERSION doesn't seem to be locatable if built static,
# so there is a bit of hack here to ignore the version on Windows
if(
    GTest_FOUND AND
    (WIN32 OR GTest_VERSION VERSION_GREATER_EQUAL ${GTEST_MIN_VERSION})
)
    enable_testing()
    add_subdirectory(test)
else()
    message(
        STATUS
        "GoogleTest >=${GTEST_MIN_VERSION} not satisfied. No tests to build"
    )
endif()
