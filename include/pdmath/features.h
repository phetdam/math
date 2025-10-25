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

// cxxabi.h (available for compilers following the Itanium C++ ABI)
#ifdef __has_include
#if __has_include(<cxxabi.h>)
#define PDMATH_ITANIUM_ABI 1
#endif  // __has_include(<cxxabi.h>)
#endif  // __has_include

#ifndef PDMATH_ITANIUM_ABI
#define PDMATH_ITANIUM_ABI 0
#endif  // PDMATH_ITANIUM_ABI

// SIMD features. see https://stackoverflow.com/a/11228864/14227825

// SSE from xmmintrin.h
#ifdef __has_include
#if __has_include(<xmmintrin.h>)
#define PDMATH_HAS_SSE 1
#endif  // __has_include(<xmmintrin.h>)
#endif  // __has_include

#ifndef PDMATH_HAS_SSE
#define PDMATH_HAS_SSE 0
#endif  // PDMATH_HAS_SSE

// SSE2 from emmintrin.h
#ifdef __has_include
#if __has_include(<emmintrin.h>)
#define PDMATH_HAS_SSE2 1
#endif  // __has_include(<emmintrin.h>)
#endif  // __has_include

#ifndef PDMATH_HAS_SSE2
#define PDMATH_HAS_SSE2 1
#endif  // PDMATH_HAS_SSE2

// SSE3 from pmmintrin.h
#ifdef __has_include
#if __has_include(<pmmintrin.h>)
#define PDMATH_HAS_SSE3 1
#endif  // __has_include(<pmmintrin.h>)
#endif  // __has_include

// SSE4.1 from smmintrin.h
#ifdef __has_include
#if __has_include(<smmintrin.h>)
#define PDMATH_HAS_SSE4_1 1
#endif  // __has_include(<smmintrin.h>)
#endif  // __has_include

#ifndef PDMATH_HAS_SSE4_1
#define PDMATH_HAS_SSE4_1 0
#endif  // PDMATH_HAS_SSE4_1

// SSE4.2 from nmmintrin.h
#ifdef __has_include
#if __has_include(<nmmintrin.h>)
#define PDMATH_HAS_SSE4_2 1
#endif  // __has_include(<nmmintrin.h>)
#endif  // __has_include

#ifndef PDMATH_HAS_SSE4_2
#define PDMATH_HAS_SSE4_2 0
#endif  // PDMATH_HAS_SSE4_2

// AVX, AVX2, FMA from immintrin.h
#ifdef __has_include
#if __has_include(<immintrin.h>)
#define PDMATH_HAS_AVX 1
#define PDMATH_HAS_AVX2 1
#define PDMATH_HAS_FMA 1
#endif  // __has_include(<immintrin.h>)
#endif  // __has_include

#ifndef PDMATH_HAS_AVX
#define PDMATH_HAS_AVX 0
#endif  // PDMATH_HAS_AVX

#ifndef PDMATH_HAS_AVX2
#define PDMATH_HAS_AVX2 0
#endif  // PDMATH_HAS_AVX2

#ifndef PDMATH_HAS_FMA
#define PDMATH_HAS_FMA 0
#endif  // PDMATH_HAS_FMA

// AVX-512 from zmmintrin.h
#ifdef __has_include
#if __has_include(<zmmintrin.h>)
#define PDMATH_HAS_AVX512 1
#endif  // __has_include(<zmmintrin.h>)
#endif  // __has_include

#ifndef PDMATH_HAS_AVX512
#define PDMATH_HAS_AVX512 0
#endif  // PDMATH_HAS_AVX512

#endif  // PDMATH_FEATURES_H_
