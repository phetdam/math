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

// SIMD features. see https://stackoverflow.com/a/28939692/14227825

// MMX
// MSVC-specific logic
#if defined(_MSC_VER)
// MSVC has no explicit MMX detection so assume SSE implies it. for x86, we
// have to have _M_IX86_FP of 1 or higher to indicate MMX support
#if defined(_M_IX86_FP)
#if _M_IX86_FP >= 1
#define PDMATH_HAS_MMX 1
#endif  // _M_IX86_FP >= 1
// for x64, SSE2 is the default instruction set, so just set to 1
#elif defined(_M_AMD64)
#define PDMATH_HAS_MMX 1
#endif  // !defined(_M_IX86_FP) && !defined(_M_AMD64)
// GCC/Clang
#elif defined(__MMX__)
#define PDMATH_HAS_MMX 1
#endif  // !defined(_MSC_VER) && !defined(__MMX__)

#ifndef PDMATH_HAS_MMX
#define PDMATH_HAS_MMX 0
#endif  // PDMATH_HAS_MMX

// SSE
// MSVC-specific logic
#if defined(_MSC_VER)
// for x86, _M_IX86_FP needs to be 1 or higher to indicate SSE support
#if defined(_M_IX86_FP)
#if _M_IX86_FP >= 1
#define PDMATH_HAS_SSE 1
#endif  // _M_IX86_FP >= 1
// for x64, SSE2 is the default instruction set, so just set to 1
#elif defined(_M_AMD64)
#define PDMATH_HAS_SSE 1
#endif  // !defined(_M_IX86_FP) && !defined(_M_AMD64)
// GCC/Clang
#elif defined(__SSE__)
#define PDMATH_HAS_SSE 1
#endif  // !defined(_MSC_VER) && !defined(__SSE__)

#ifndef PDMATH_HAS_SSE
#define PDMATH_HAS_SSE 0
#endif  // PDMATH_HAS_SSE

// SSE2
// MSVC-specific logic
#if defined(_MSC_VER)
// for x86, _M_IX86_FP needs to be 2 or higher to indicate SSE2 support
#if defined(_M_IX86_FP)
#if _M_IX86_FP >= 2
#define PDMATH_HAS_SSE2 1
#endif  // _M_IX86_FP >= 2
// for x64, SSE2 is the default instruction set, so just set to 1
#elif defined(_M_AMD64)
#define PDMATH_HAS_SSE2 1
#endif  // !defined(_M_IX86_FP) && !defined(_M_AMD64)
// GCC/Clang
#elif defined(__SSE2__)
#define PDMATH_HAS_SSE2 1
#endif  // !defined(_MSC_VER) && !defined(__SSE2__)

#ifndef PDMATH_HAS_SSE2
#define PDMATH_HAS_SSE2 1
#endif  // PDMATH_HAS_SSE2

// SSE3
// MSVC has no explicit SSE3 detection so assume AVX implies it
#if defined(__SSE3__) || (defined(_MSC_VER) && defined(__AVX__))
#define PDMATH_HAS_SSE3 1
#else
#define PDMATH_HAS_SSE3 0
#endif  // !defined(__SSE3__) && !(defined(_MSC_VER) && defined(__AVX__))

// SSE4.1
// MSVC has no explicit SSE4.1 detection so assume AVX implies it
#if defined(__SSE4_1__) || (defined(_MSC_VER) && defined(__AVX__))
#define PDMATH_HAS_SSE4_1 1
#else
#define PDMATH_HAS_SSE4_1 0
#endif  // !defined(__SSE4_1__) && !(defined(_MSC_VER) && defined(__AVX__))

// SSE4.2
// MSVC has no explicit SSE4.1 detection so assume AVX implies it
#if defined(__SSE4_2__) || (defined(_MSC_VER) && defined(__AVX__))
#define PDMATH_HAS_SSE4_2 1
#else
#define PDMATH_HAS_SSE4_2 0
#endif  // !defined(__SSE4_2__) && !(defined(_MSC_VER) && defined(__AVX__))

// AVX
#if defined(__AVX__)
#define PDMATH_HAS_AVX 1
#else
#define PDMATH_HAS_AVX 0
#endif  // !defined(__AVX__)

// AVX2
#if defined(__AVX2__)
#define PDMATH_HAS_AVX2 1
#else
#define PDMATH_HAS_AVX2 0
#endif  // !defined(__AVX2__)

// FMA
// MSVC has no explicit FMA detection so assume AVX implies it
#if defined(__FMA__) || (defined(_MSC_VER) && defined(__AVX__))
#define PDMATH_HAS_FMA 1
#else
#define PDMATH_HAS_FMA 0
#endif  // !defined(__FMA__) && !(defined(_MSC_VER) && defined(__AVX__))

// AVX-512BW
#if defined(__AVX512BW__)
#define PDMATH_HAS_AVX512BW 1
#else
#define PDMATH_HAS_AVX512BW 0
#endif  // !defined(__AVX512BW__)

// AVX-512CD
#if defined(__AVX512CD__)
#define PDMATH_HAS_AVX512CD 1
#else
#define PDMATH_HAS_AVX512CD 0
#endif  // !defined(__AVX512CD__)

// AVX-512DQ
#if defined(__AVX512DQ__)
#define PDMATH_HAS_AVX512DQ 1
#else
#define PDMATRH_HAS_AVX512DQ 0
#endif  // !defined(__AVX512DQ__)

// AVX-512F
#if defined(__AVX512F__)
#define PDMATH_HAS_AVX512F 1
#else
#define PDMATH_HAS_AVX512F 0
#endif  // !defined(__AVX512F__)

// AVX-512VL
#if defined(__AVX512VL__)
#define PDMATH_HAS_AVX512VL 1
#else
#define PDMATH_HAS_AVX512VL 0
#endif  // !defined(__AVX512VL__)

#endif  // PDMATH_FEATURES_H_
