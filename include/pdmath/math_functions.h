/**
 * @file math_functions.h
 * @author Derek Huang
 * @brief C++ header for some mathematical function implementations
 * @copyright MIT License
 */

#ifndef PDMATH_MATH_FUNCTIONS_H_
#define PDMATH_MATH_FUNCTIONS_H_

#include <type_traits>

#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * Himmelblau's function.
 *
 * The return type of this function is the common type of `T` and `U`. If the
 * common type is integral, the result type is `int` or wider.
 *
 * @tparam T Arithmetic type
 * @tparam U Arithmetic type
 */
template <typename T, typename U>
constexpr auto himmelblau(
  T x,
  U y,
  constraint_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>> = 0)
{
#if 0
  // original implementation for posterity
  return
    std::pow(std::pow(x, 2) + y - 11, 2) +
    std::pow(x + std::pow(y, 2) - 7, 2);
#endif  // 0
  //
  // note:
  //
  // we don't use std::pow since the MSVC implementation uses a generic
  // std::pow template defined by _GENERIC_MATH2_BASE where the return type is
  // _Common_float_type_t<T, U>, which defaults to double if one of T or U is
  // *not* a floating-point type. therefore, with MSVC, the following decltype
  // decltype(himmelblau(float{}, int{})) is actually double, *not* float.
  //
  // by using built-in math operators we ensure all arithmetic conversions
  // occur as expected. however, we have not tested if this is slower. the
  // current assumption is that with optimization enabled, the compiler should
  // be able to use intrinsics or extra optimizations where possible.
  //
  auto p1 = x * x + y - 11;
  auto p2 = x + y * y - 7;
  return p1 * p1 + p2 * p2;
}

/**
 * Three-hump camel function.
 *
 * The return type of this function is the common type of `T` and `U` if this
 * common type is a floating-point type and is `double` otherwise.
 *
 * @tparam T Arithmetic type
 * @tparam U Arithmetic type
 */
template <typename T, typename U>
constexpr auto three_hump_camel(
  T x,
  U y,
  constraint_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U>> = 0)
{
#if 0
  // original implementation for posterity
  return
    2 * std::pow(x, 2) - 1.05 * std::pow(x, 4) +
    std::pow(x, 6) / 6. + x * y + std::pow(y, 2);
#endif  // 0
  // common type of T and U
  using common_type = std::common_type_t<T, U>;
  // value type is double if common_type is not floating point
  using value_type = std::conditional_t<
    std::is_floating_point_v<common_type>, common_type, double
  >;
  // note: constants use value_type to ensure correct types are used
  // note: see himmelblau() for explanation on why std::pow is not used
  // note: manual x * ... * x used since floating math is not associative
  auto p1 = 2 * x * x;
  auto p2 = static_cast<value_type>(1.05) * x * x * x * x;
  auto p3 = x * x * x * x * x / static_cast<value_type>(6.);
  auto p4 = x * y;
  auto p5 = y * y;
  return p1 - p2 + p3 + p4 + p5;
}

}  // namespace pdmath

#endif  // PDMATH_MATH_FUNCTIONS_H_
