/**
 * @file features.h
 * @author Derek Huang
 * @brief C/C++ header for compiler/library feature detection
 * @copyright MIT License
 */

#ifndef PDMATH_FEATURES_H_
#define PDMATH_FEATURES_H_

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

#endif  // PDMATH_FEATURES_H_
