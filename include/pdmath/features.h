/**
 * @file features.h
 * @author Derek Huang
 * @brief C/C++ header for compiler/library feature detection
 * @copyright MIT License
 */

#ifndef PDMATH_FEATURES_H_
#define PDMATH_FEATURES_H_

#include "pdmath/common.h"

// C++ standards support
#if defined(PDMATH_CPLUSPLUS)
// C++17
#if PDMATH_CPLUSPLUS >= 201703L
#define PDMATH_HAS_CXX17 1
#endif  // PDMATH_CPLUSPLUS >= 201703L
// C++20
#if PDMATH_CPLUSPLUS >= 202002L
#define PDMATH_HAS_CXX20 1
#endif  // PDMATH_CPLUSPLUS >= 202002L
// C++23
#if PDMATH_CPLUSPLUS >= 202302L
#define PDMATH_HAS_CXX23 1
#endif  // PDMATH_CPLUSPLUS >= 202302L
#endif  // defined(PDMATH_CPLUSPLUS)

#ifndef PDMATH_HAS_CXX17
#define PDMATH_HAS_CXX17 0
#endif  // PDMATH_HAS_CXX17

#ifndef PDMATH_HAS_CXX20
#define PDMATH_HAS_CXX20 0
#endif  // PDMATH_HAS_CXX20

#ifndef PDMATH_HAS_CXX23
#define PDMATH_HAS_CXX23 0
#endif  // PDMATH_HAS_CXX23

// Eigen 3
#ifdef __has_include
// check Eigen 3 signature for surest check
#if __has_include(<eigen3/signature_of_eigen3_matrix_library>)
#define PDMATH_HAS_EIGEN3 1
#endif  // __has_include(<eigen3/signature_of_eigen3_matrix_library>)
#endif  // __has_include

#ifndef PDMATH_HAS_EIGEN3
#define PDMATH_HAS_EIGEN3 0
#endif  // PDMATH_HAS_EIGEN3

// Boost headers
#ifdef __has_include
#if __has_include(<boost/version.hpp>)
#define PDMATH_HAS_BOOST 1
#endif  // __has_include(<boost/version.hpp>)
#endif  // __has_include

#ifndef PDMATH_HAS_BOOST
#define PDMATH_HAS_BOOST 0
#endif  // PDMATH_HAS_BOOST

// libquadmath
#ifdef __has_include
#if __has_include(<quadmath.h>)
#define PDMATH_HAS_QUADMATH 1
#endif  // __has_include(<quadmath.h>)
#endif  // __has_include

#ifndef PDMATH_HAS_QUADMATH
#define PDMATH_HAS_QUADMATH 0
#endif  // PDMATH_HAS_QUADMATH

#endif  // PDMATH_FEATURES_H_
