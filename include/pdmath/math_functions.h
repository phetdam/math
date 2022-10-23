/**
 * @file math_functions.h
 * @author Derek Huang
 * @brief C++ header for some mathematical function implementations
 * @copyright MIT License
 */

#ifndef PDMATH_MATH_FUNCTIONS_H_
#define PDMATH_MATH_FUNCTIONS_H_

#include <cmath>

#include "pdmath/warnings.h"

namespace pdmath {

/**
 * Himmelblau's function.
 *
 * @tparam T scalar type
 */
template <typename T>
inline T himmelblau(const T& x, const T& y)
{
// disable warning about conversion from double to T
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4244)
#endif  // _MSC_VER
  return
    std::pow(std::pow(x, 2) + y - 11, 2) +
    std::pow(x + std::pow(y, 2) - 7, 2);
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
}

/**
 * Three-hump camel function.
 *
 * @tparam T scalar type
 */
template <typename T>
inline T three_hump_camel(const T& x, const T& y)
{
// disable warning about conversion from double to T
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4244)
#endif  // _MSC_VER
  return
    2 * std::pow(x, 2) - 1.05 * std::pow(x, 4) +
    std::pow(x, 6) / 6. + x * y + std::pow(y, 2);
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
}

}  // namespace pdmath

#endif  // PDMATH_MATH_FUNCTIONS_H_
