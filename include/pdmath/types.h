/**
 * @file types.h
 * @author Derek Huang
 * @brief C++ header for some user-defined types/type aliases
 * @copyright MIT License
 */

#ifndef PDMATH_TYPES_H_
#define PDMATH_TYPES_H_

#include <array>
#include <utility>
#include <vector>

#include <Eigen/Core>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

namespace pdmath {

using double_vector = std::vector<double>;
using float_vector = std::vector<float>;

template <typename T>
using boost_vector = boost::numeric::ublas::vector<T>;
template <typename T>
using boost_matrix = boost::numeric::ublas::matrix<T>;

template <typename T>
using array_pair = std::array<T, 2>;

}  // namespace pdmath

#endif  // PDMATH_TYPES_H_
