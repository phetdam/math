/**
 * @file types.h
 * @author Derek Huang
 * @brief C++ header for some user-defined types/type aliases
 * @copyright MIT License
 */

#ifndef PDMATH_TYPES_H_
#define PDMATH_TYPES_H_

#include <array>
#include <cstddef>
#include <utility>
#include <vector>

#include "pdmath/features.h"

#if PDMATH_HAS_BOOST
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#endif  // PDMATH_HAS_BOOST

namespace pdmath {

using vector_d = std::vector<double>;
using vector_f = std::vector<float>;

// note: PDMATH_HAS_BOOST should be used to check if the Boost aliases exist
#if PDMATH_HAS_BOOST
template <typename T>
using boost_vector = boost::numeric::ublas::vector<T>;
using boost_vector_d = boost_vector<double>;
using boost_vector_f = boost_vector<float>;
template <typename T>
using boost_matrix = boost::numeric::ublas::matrix<T>;
using boost_matrix_d = boost_matrix<double>;
using boost_matrix_f = boost_matrix<float>;
#endif  // PDMATH_HAS_BOOST

template <std::size_t N>
using array_d = std::array<double, N>;
template <std::size_t N>
using array_f = std::array<float, N>;
template <typename T>
using array_pair = std::array<T, 2>;
using array_pair_d = array_pair<double>;
using array_pair_f = array_pair<float>;
template <typename T>
using array_triple = std::array<T, 3>;
using array_triple_d = array_triple<double>;
using array_triple_f = array_triple<float>;

}  // namespace pdmath

#endif  // PDMATH_TYPES_H_
