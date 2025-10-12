/**
 * @file fpi.cc
 * @author Derek Huang
 * @brief C++ program that prints \f$\pi\f$ in various floating precisions
 * @copyright MIT License
 */

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <utility>

#include "pdmath/common.h"
#include "pdmath/features.h"

#if PDMATH_HAS_QUADMATH
#include <quadmath.h>
#endif  // PDMATH_HAS_QUADMATH

namespace {

// first 50 digits of pi as double, float, and long double
#define PI_50 3.14159265358979323846264338327950288419716939937510
#define PI_50F PDMATH_CONCAT(PI_50, f)
#define PI_50L PDMATH_CONCAT(PI_50, L)
// if quadmath.h is available, we can print __float128, so also include it
#if PDMATH_HAS_QUADMATH
#define PI_50Q PDMATH_CONCAT(PI_50, Q)
#endif  // PDMATH_HAS_QUADMATH

// first 50 digits of pi as a string literal + number of digits
constexpr const char pi_str[] = PDMATH_STRINGIFY(PI_50);
constexpr auto pi_digits = std::size(pi_str) - 3u;  // subtract '3', '.', '\0'

/**
 * Implementation for a template type the represents an array of spaces.
 *
 * This takes a `std::index_sequence<Is...>` to enable indexing.
 *
 * @tparam T `std::index_sequence<Is...>`
 */
template <typename T>
struct spaces_impl {};

/**
 * Specialization for `std::make_index_sequence<0u>` with an empty string.
 */
template <>
struct spaces_impl<std::index_sequence<>> {
  static constexpr const char value[] = {'\0'};
};

/**
 * Specialization for `std::make_index_sequence<1u>`.
 */
template <>
struct spaces_impl<std::index_sequence<0u>> {
  static constexpr const char value[] = {' ', '\0'};
};

/**
 * Partial specialization for `std::make_index_sequence<sizeof...(Is) + 1u>`.
 *
 * @tparam I First index (zero)
 * @tparam Is Index pack 1 through `sizeof...(Is)`
 */
template <std::size_t I, std::size_t... Is>
struct spaces_impl<std::index_sequence<I, Is...>> {
  static constexpr const char value[] = {
    ' ',
    // since Is starts with 1 we have to shift down by 1
    spaces_impl<std::index_sequence<(Is - 1u)...>>::value[Is - 1u]...,
    '\0'
  };
};

/**
 * Template type that represents an array of spaces.
 *
 * The internal value is constructed recursively via pack indexing.
 *
 * @tparam N Number of spaces
 */
template <std::size_t N>
struct spaces : spaces_impl<std::make_index_sequence<N>> {};

/**
 * Get the `spaces<N>` with `N` equal to `std::numeric_limits<T>::digits10`.
 *
 * @tparam T type
 */
template <typename T>
using spaces10 = spaces<std::numeric_limits<T>::digits10>;

}  // namespace

int main()
{
  // printf requires int for * with specifier, so cast pi_digits
  constexpr auto pi_digits_i = static_cast<int>(pi_digits);
  // print various values of the first 50 digits of pi
  std::printf("pi50       string: %s\n", pi_str);
  std::printf("pi50        float: %1.*f\n", pi_digits_i, PI_50F);
  std::printf("                     %s^\n", spaces10<decltype(PI_50F)>::value);
  std::printf("pi50       double: %1.*f\n", pi_digits_i, PI_50);
  std::printf("                     %s^\n", spaces10<decltype(PI_50)>::value);
  std::printf("pi50  long double: %1.*Lf\n", pi_digits_i, PI_50L);
  std::printf("                     %s^\n", spaces10<decltype(PI_50L)>::value);
// need quadmath_snprintf to print out __float128
#if PDMATH_HAS_QUADMATH
  char pi_q_str[sizeof pi_str];
  quadmath_snprintf(pi_q_str, sizeof pi_q_str, "%1.*Qf", pi_digits_i, PI_50Q);
  std::printf("pi50   __float128: %s\n", pi_q_str);
  // note: FLT128_DIG defined in quadmath.h
  std::printf("                     %s^\n", spaces<FLT128_DIG>::value);
#endif  // PDMATH_HAS_QUADMATH
  return EXIT_SUCCESS;
}
