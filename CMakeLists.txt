cmake_minimum_required(VERSION 3.21)

project(
    pdmath
    VERSION 0.1.0
    DESCRIPTION "Independent mathetmatical work"
    HOMEPAGE_URL "https://github.com/phetdam/math"
    LANGUAGES CXX
)

# for releases build info is empty
option(PDMATH_BUILD_RELEASE "Build a pdmath release" OFF)

# locate Git for build hash
find_package(Git 2.34)
if(Git_FOUND)
    message(STATUS "Git version: ${GIT_VERSION_STRING}")
else()
    message(STATUS "Git version: None")
endif()

# check if generator is multi-config or not
get_property(PDMATH_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(PDMATH_IS_MULTI_CONFIG)
    message(STATUS "Generator config: Multi")
else()
    message(STATUS "Generator config: Single")
endif()

# use Debug config for single-config generators
if(PDMATH_IS_MULTI_CONFIG)
    message(STATUS "Build config: Multi")
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
    message(STATUS "Build config: ${CMAKE_BUILD_TYPE}")
endif()

# indicate if unity build is used or not
# TODO: this doesn't really help. the bottleneck is in linking some of the
# executables (may be related to Boost or Eigen being really heavy)
if(CMAKE_UNITY_BUILD)
    message(STATUS "Build unity: Yes")
else()
    message(STATUS "Build unity: No")
endif()

include(CTest)

# update CMake module path + include project modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(pdmath_latex_compile)
include(pdmath_termcolor)

# TODO: consider upgrading to C++20
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# TODO: consider making Boost and Eigen functionality optional

# FindBoost.cmake policy
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.30)
    cmake_policy(PUSH)
    cmake_policy(SET CMP0167 OLD)
endif()

# Boost
find_package(Boost 1.71.0 REQUIRED)
message(STATUS "Boost version: ${Boost_VERSION}")

# FindBoost.cmake policy
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.30)
    cmake_policy(POP)
endif()

# Eigen
# https://eigen.tuxfamily.org/dox/TopicCMakeGuide.html indicates NO_MODULE
# (equivalent to CONFIG) should be specified
find_package(Eigen3 3.4 REQUIRED CONFIG)
message(STATUS "Eigen version: ${Eigen3_VERSION}")

# Google Test
set(GTEST_MIN_VERSION 1.10.0)
find_package(GTest ${GTEST_MIN_VERSION})
if(GTest_FOUND)
    message(STATUS "Google Test version: ${GTest_VERSION}")
else()
    message(STATUS "Google Test version: None")
endif()

# LaTeX components
find_package(LATEX COMPONENTS PDFLATEX BIBTEX)
if(LATEX_FOUND)
    # get pdflatex version
    execute_process(
        COMMAND ${PDFLATEX_COMPILER} -version
        RESULT_VARIABLE pdflatex_res
        OUTPUT_VARIABLE pdflatex_out
        ERROR_VARIABLE pdflatex_err
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    # if failed, fatal
    if(pdflatex_res)
        message(FATAL_ERROR "pdflatex -version failed: ${pdflatex_err}")
    endif()
    unset(pdflatex_res)
    unset(pdflatex_err)
    # get first line of pdflatex -version output
    string(REGEX REPLACE "^([^\n]+)\n.+$" "\\1" PDFLATEX_VERSION "${pdflatex_out}")
    unset(pdflatex_out)
    # get version info. format is pdfTeX <version> ...
    string(
        REGEX REPLACE "^[a-zA-Z]+[ ]+([0-9.-]+)[ ]+.+$" "\\1"
        PDFLATEX_VERSION "${PDFLATEX_VERSION}"
    )
    message(STATUS "pdfTeX version: ${PDFLATEX_VERSION}")
    # get BibTeX version
    execute_process(
        COMMAND ${BIBTEX_COMPILER} -version
        RESULT_VARIABLE bibtex_res
        OUTPUT_VARIABLE bibtex_out
        ERROR_VARIABLE bibtex_err
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    # if failed, fatal
    if(bibtex_res)
        message(FATAL_ERROR "bibtex -version failed: ${bibtex_err}")
    endif()
    unset(bibtex_res)
    unset(bibtex_err)
    # get first line of bibtex -version output
    string(REGEX REPLACE "^([^\n]+)\n.+$" "\\1" BIBTEX_VERSION "${bibtex_out}")
    unset(bibtex_out)
    # get version info. format is BibTeX <version> ...
    string(
        REGEX REPLACE "^[a-zA-Z]+[ ]+([0-9.-a-z]+)[ ]+.+$" "\\1"
        BIBTEX_VERSION "${BIBTEX_VERSION}"
    )
    message(STATUS "BibTeX version: ${BIBTEX_VERSION}")
else()
    message(STATUS "pdfTeX version: None")
    message(STATUS "BibTeX version: None")
endif()

# find Doxygen
# TODO: Doxygen 1.11.0 has a regression with Markdown file \ref commands not
# properly resolving references. may want to add a warning about this
find_package(Doxygen 1.10)
if(Doxygen_FOUND)
    message(STATUS "Doxygen version: ${DOXYGEN_VERSION}")
else()
    message(STATUS "Doxygen version: None")
endif()

# find GCC's libquadmath library (for fpi program)
message(CHECK_START "Looking for libquadmath")
# quadmath.h is located in GCC's own include tree so we cannot locate it via
# find_path or find_file. we therefore need to try compiling and linking a
# program that includes quadmath.h and requires -lquadmath
try_compile(
    quadmath_FOUND
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/quadmath.cc
    LINK_LIBRARIES quadmath
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ${CMAKE_CXX_STANDARD_REQUIRED}
)
if(quadmath_FOUND)
    message(CHECK_PASS "found")
else()
    message(CHECK_FAIL "not found")
endif()

# debug and release flags, handle MSVC case specifically
if(MSVC)
    add_compile_options(
        /Wall
        /wd4265
        /wd4514
        /wd4625
        /wd4626
        # C4710: function marked for inline expansion not inlined
        /wd4710
        # C4711: function selected for inline expansion
        /wd4711
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

# use flat build output layout with per-config subdirectories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# base include for this repo
set(PDMATH_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# configure version info. build suffix determined via Git
if(Git_FOUND AND NOT PDMATH_BUILD_RELEASE)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        RESULT_VARIABLE git_rev_res
        OUTPUT_VARIABLE git_rev_out
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # if failed, warn and use DEV for build info
    if(git_ver_res)
        message(WARNING "git rev-parse --short HEAD error! build info set to DEV")
        set(PDMATH_BUILD_INFO "DEV")
    # otherwise, use the output
    else()
        set(PDMATH_BUILD_INFO ${git_rev_out})
    endif()
    # clean up
    unset(git_rev_res)
    unset(git_rev_out)
endif()
# if PDMATH_BUILD_INFO is not defined, set to DEV if not building release
if(NOT DEFINED PDMATH_BUILD_INFO)
    if(PDMATH_BUILD_RELEASE)
        set(PDMATH_BUILD_INFO "")
    else()
        set(PDMATH_BUILD_INFO "DEV")
    endif()
endif()
# standard version components
foreach(comp MAJOR MINOR PATCH)
    set(PDMATH_${comp}_VERSION ${PROJECT_VERSION_${comp}})
endforeach()
# set full version with build info. non-release appends build info
set(
    PDMATH_VERSION
    ${PDMATH_MAJOR_VERSION}.${PDMATH_MINOR_VERSION}.${PDMATH_PATCH_VERSION}
)
if(NOT PDMATH_BUILD_RELEASE)
    string(APPEND PDMATH_VERSION "-${PDMATH_BUILD_INFO}")
endif()

# print version + configure version.h.in
message(STATUS "Build version: ${PDMATH_VERSION}")
set(pdmath_version_h include/pdmath/version.h)
# variables consumed:
#
#   PDMATH_MAJOR_VERSION
#   PDMATH_MINOR_VERSION
#   PDMATH_PATCH_VERSION
#   PDMATH_VERSION
#   PDMATH_BUILD_INFO
#
configure_file(${pdmath_version_h}.in ${pdmath_version_h} @ONLY NEWLINE_STYLE LF)
message(STATUS "Generated ${pdmath_version_h}")

# main pdmath INTERFACE target since library is header-only
# TODO: technically this library depends on Boost and Eigen headers so the
# relevant targets should be linked as INTERFACE requirements. however, in the
# future we want to make the dependency optional, so we exclude for now
add_library(pdmath INTERFACE)
target_include_directories(
    pdmath INTERFACE
    $<BUILD_INTERFACE:${PDMATH_INCLUDE}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# TODO: create pdmath::pdmath INTERFACE target. if you don't do this, every
# time include directories are updated, all targets will rebuild
include_directories(${PDMATH_INCLUDE} ${CMAKE_CURRENT_BINARY_DIR}/include)

add_subdirectory(examples)
add_subdirectory(src)

# enable building of TeX to PDF is pdflatex + bibtex are available
if(LATEX_FOUND)
    add_subdirectory(tex)
else()
    message(STATUS "Skipping TeX (pdfLaTeX + BibTeX required)")
endif()

# Google Test is required for testing
if(GTest_FOUND)
    add_subdirectory(test)
else()
    message(STATUS "Skipping tests (Google Test + Google Mock required)")
endif()
