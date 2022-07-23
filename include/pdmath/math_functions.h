/**
 * @file math_functions.h
 * @author Derek Huang
 * @brief C++ header for some mathematical function implementations
 * @copyright MIT License
 */

#ifndef PDMATH_MATH_FUNCTIONS_H_
#define PDMATH_MATH_FUNCTIONS_H_

#include <cmath>

namespace pdmath {

/**
 * Himmelblau's function.
 *
 * @tparam T scalar type
 */
template <typename T>
T himmelblau(const T& x, const T& y)
{
  return
    std::pow(std::pow(x, 2) + y - 11, 2) +
    std::pow(x + std::pow(y, 2) - 7);
}

/**
 * Three-hump camel function.
 *
 * @tparam T scalar type
 */
template <typename T>
T three_hump_camel(const T& x, const T& y)
{
  return
    2 * std::pow(x, 2) - T(1.05) * std::pow(x, 4) +
    std::pow(x, 6) / T(6.) + x * y + std::pow(y, 2);
}

}  // namespace pdmath

#endif  // PDMATH_MATH_FUNCTIONS_H_
