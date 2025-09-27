cmake_minimum_required(VERSION 3.21)

project(
    pdmath
    VERSION 0.1.0
    DESCRIPTION "Independent mathetmatical work"
    HOMEPAGE_URL "https://github.com/phetdam/math"
    LANGUAGES CXX
)

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

include(CTest)

# update CMake module path + include
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(pdmath_compile_latex)

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

# TODO: create pdmath::pdmath INTERFACE target
include_directories(${PDMATH_INCLUDE})

add_subdirectory(src)
add_subdirectory(examples)
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
