/**
 * @file pi.h
 * @author Derek Huabg
 * @brief C++ header for pi constants of varying precision.
 * @copyright MIT License
 */

#ifndef PDMATH_PI_H_
#define PDMATH_PI_H_

#include <cstddef>
#include <type_traits>

#include "pdmath/features.h"
#include "pdmath/type_traits.h"
#include "pdmath/warnings.h"

// SIMD headers
#if PDMATH_HAS_AVX
#include <immintrin.h>
#endif  // PDMATH_HAS_AVX

namespace pdmath {

/**
 * Traits class for \f$\pi\f$.
 *
 * We use a traits class so we can have different values of \f$\pi\f$ for
 * varying levels of floating-point precision. The traits stores the value of
 * \f$\pi\f$ to an appropriate precision as well as the number of digits.
 *
 * The first 50 digits of \f$\pi\f$ are shown below for comparison.
 *
 * \f[3.14159265358979323846264338327950288419716939937510\f]
 *
 * Values used are from https://stackoverflow.com/a/507879/14227825.
 *
 * @note Each constant is truncated at the number of digits that can be
 *  preserved without change due to rounding mode. Therefore, each `digits`
 *  member is the same as `std::numeric_limits<T>::digits10` for `T`.
 *
 * @tparam T Floating-point type
 */
template <typename T>
struct pi_traits {};

/**
 * \f$\pi\f$ traits for `float`.
 */
template <>
struct pi_traits<float> {
  static constexpr auto value = 3.141592f;
  static constexpr auto digits = 6u;
};

/**
 * \f$\pi\f$ traits for `double`.
 */
template <>
struct pi_traits<double> {
  static constexpr auto value = 3.141592653589793;
  static constexpr auto digits = 15u;
};

/**
 * \f$\pi\f$ traits for `long double`.
 *
 * With GCC `__SIZEOF_LONG_DOUBLE__` is used while for other compilers like
 * MSVC the `long double` values are the same as for `double`.
 *
 * @note It it not recommended to use `long double` for portable results since
 *  the type width varies across platforms.
 */
template <>
struct pi_traits<long double> {
// use extended precision on GCC if possible
#if defined(__SIZEOF_LONG_DOUBLE__)
// 80-bit Intel x87
#if __SIZEOF_LONG_DOUBLE__ * CHAR_BIT >= 80
  static constexpr auto value = 3.141592653589793238L;
  static constexpr auto digits = 18u;
// 128-bit quad precision
#elif __SIZEOF_LONG_DOUBLE__ * CHAR_BIT >= 128
  static constexpr auto value = 3.141592653589793238462643383279502Q;
  static constexpr auto digits = 33u;
#endif
// default to 64-bit double
#else
  static constexpr auto value = 3.141592653589793L;
  static constexpr auto digits = 15u;
#endif  // !defined(__SIZEOF_LONG_DOUBLE__)
};

/**
 * \f$\pi\f$ template.
 *
 * @tparam T Floating-point type
 */
template <typename T>
constexpr T pi_v = pi_traits<T>::value;

/**
 * \f$\pi\f$ double-precision constant.
 */
inline constexpr auto pi = pi_v<double>;

namespace detail {

/**
 * Indicate if a point is inside the 2D closed unit circle \f$[-1, 1]^2\f$.
 *
 * @tparam T Floating-point type
 * @tparam U Floating-point type
 *
 * @param x First dimension value
 * @param y Second dimension value
 */
template <typename T, typename U>
bool in_unit_circle(
  T x,
  U y,
  constraint_t<std::is_floating_point_v<T> && std::is_floating_point_v<U>> = 0)
{
  return x * x + y * y <= 1;  // note: no square root operation is needed
}

#if PDMATH_HAS_AVX
/**
 * Indicate if points are inside the closed unit circle \f$[-1, 1]^2\f$.
 *
 * This uses AVX intrinsics to return a mask of the points in the circle.
 *
 * @param x First dimension values
 * @param y Second dimension values
 */
inline auto in_unit_circle(__m256 x, __m256 y)
{
  // prod = x * x
  auto prod = _mm256_mul_ps(x, x);
  // prod += y * y
  prod = _mm256_fmadd_ps(y, y, prod);
  // check prod <= 1
  __m256 one = {1, 1, 1, 1, 1, 1, 1, 1};
  return _mm256_cmp_ps_mask(prod, one, _CMP_LE_OQ);
}
#endif  // PDMATH_HAS_AVX

}  // namespace detail

/**
 * Estimate \f$\pi\f$ using a quasi Monte Carlo stratified sampling method.
 *
 * This aims to efficiently estimate \f$\pi\f$ using an evenly-spaced grid of
 * points that partitions \f$[0, 1]^2\f$ into evenly-size squares and then
 * samples the center point of each square. Therefore, given \f$n\f$ partitions
 * of \f$[0, 1]\f$, there will be \f$n^2\f$ squares, and the \f$(i, j)\f$
 * point, where \f$i, j \in \{0, \ldots n - 1\}\f$, will have coordinates
 * \f$\left(\frac{i + 1/2}{n}, \frac{j + 1/2}{n})\f$.
 *
 * By using this evenly-spaced sampling method we can sample a given number of
 * points far faster than using a PRNG or another sampling method. We also
 * avoid any patterning that can arise from PRNG usage.
 *
 * @note The pdmpmt project implements the same in `pdmpmt::quasi_mcpi()`.
 *
 * @tparam T Floating-point type
 *
 * @param n Number of partitions along an axis (square root of total samples)
 */
template <typename T = double>
T qmc_pi(std::size_t n, constraint_t<std::is_floating_point_v<T>> = 0) noexcept
{
  // number of points inside top-right quadrant of unit circle
  std::size_t n_in = 0u;
// MSVC emits C5219 complaining about loss of data converting size_t to T
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5219)
  // double loop
  for (decltype(n) i = 0u; i < n; i++)
    for (decltype(n) j = 0u; j < n; j++)
      n_in += detail::in_unit_circle((i + T{0.5}) / n, (j + T{0.5}) / n);
PDMATH_MSVC_WARNINGS_POP()
  // return pi estimate
  // note: second static_cast<T> is unnecessary but is there to silence MSVC
  return 4 * (static_cast<T>(n_in) / static_cast<T>(n * n));
}

}  // namespace pdmath

#endif  // PDMATH_PI_H_
