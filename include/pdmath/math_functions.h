/**
 * @file math_functions.h
 * @author Derek Huang
 * @brief C++ header for some mathematical function implementations
 * @copyright MIT License
 */

#ifndef PDMATH_MATH_FUNCTIONS_H_
#define PDMATH_MATH_FUNCTIONS_H_

#include <cmath>
#include <type_traits>

#include "pdmath/type_traits.h"
#include "pdmath/warnings.h"

namespace pdmath {

/**
 * Himmelblau's function.
 *
 * @tparam T Arithmetic type
 * @tparam U Arithmetic type
 */
template <typename T, typename U>
auto himmelblau(
  T x,
  U y,
  constraint_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>> = 0)
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
 * @tparam T Arithmetic type
 * @tparam U Arithmetic type
 */
template <typename T, typename U>
auto three_hump_camel(
  T x,
  U y,
  constraint_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>> = 0)
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
