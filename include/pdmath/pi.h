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
#include "pdmath/impl_policy.h"
#include "pdmath/simd.h"
#include "pdmath/type_traits.h"
#include "pdmath/warnings.h"

// note: can reduce the ISA requirement as necessary later
#if PDMATH_HAS_AVX2
#include <immintrin.h>
#endif  // PDMATH_HAS_AVX2

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

#if PDMATH_HAS_AVX && PDMATH_HAS_FMA
/**
 * Indicate if points are inside the closed unit circle \f$[-1, 1]^2\f$.
 *
 * This uses AVX intrinsics to return a `__m256` mask of points in the circle,
 * e.g. `0xFFFFFFFF` for selected lanes and `0x00000000` for unselected lanes.
 *
 * @param x First dimension values
 * @param y Second dimension values
 */
inline auto in_unit_circle(__m256 x, __m256 y)
{
  // prod = y * y + x * x
  auto prod = _mm256_fmadd_ps(y, y, _mm256_mul_ps(x, x));
  // check prod <= 1
  return _mm256_cmp_ps(prod, _mm256_set1_ps(1.f), _CMP_LE_OQ);
}
#endif  // PDMATH_HAS_AVX && PDMATH_HAS_FMA

#if PDMATH_HAS_AVX512F
/**
 * Indicate if points are inside the closed unit circle \f$[-1, 1]^2\f$.
 *
 * This uses AVX-512F intrinsics to return a `__mmask16` bitmask of points in
 * the circle that can be used with `_mm512_mask_blend_ps`.
 *
 * @param x First dimension values
 * @param y Second dimension values
 */
inline auto in_unit_circle(__m512 x, __m512 y)
{
  // prod = y * y + x * x
  auto prod = _mm512_fmadd_ps(y, y, _mm512_mul_ps(x, x));
  // check prod <= 1
  return _mm512_cmple_ps_mask(prod, _mm512_set1_ps(1.f));
}
#endif  // PDMATH_HAS_AVX512F

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

// TODO: should we also check for SSE3 and MMX?
#if PDMATH_HAS_AVX2
/**
 * Estimate \f$\pi\f$ using a quasi Monte Carlo stratified sampling method.
 *
 * This is a SIMD implementation of the original serial logic that uses AVX2
 * instructions to speed up computation. With a 64-bit /arch:AVX2 MSVC build
 * this results in around a 3.4x speedup while with a 64-bit GCC -march=native
 * build on an AVX2 machine this results in nearly a 7x speedup.
 *
 * @tparam T Floating-point type
 *
 * @param n Number of partitions along an axis (square root of total samples)
 */
template <typename T = double>
T qmc_pi(
  simd_implementation,
  std::size_t n,
  constraint_t<std::is_floating_point_v<T>> = 0) noexcept
{
// disable C5219 which is emitted in several places
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5219)
  // number of elements in an __m256
  constexpr auto stride = sizeof(__m256) / sizeof(float);
  // TODO: move to SIMD traits header?
  // SIMD points inside top-right quadrant of unit circle
  auto pn_in = _mm256_set1_epi32(0);
  // scalar count of points inside top-right quadrant of unit circle
  std::size_t n_in = 0u;
  // packed 1.f / n precomputed for FMA
  auto pn_inv = _mm256_set1_ps(1.f / n);
  // packed step 0.5, ... (stride - 0.5) / n values precomputed for FMA
  auto pstep = _mm256_mul_ps(pn_inv, iota<__m256>(0.5f));
  // outer loop
  for (decltype(n) i = 0u; i < n; i++) {
    // inner loop counter. this is used by both strided and scalar loop
    decltype(n) j = 0u;
    // since i is fixed for the inner loop, compute packed (i + 0.5f) / n
    auto px = _mm256_set1_ps((i + 0.5f) / n);
    // strided inner loop
    for (; j + stride <= n; j += stride) {
      // use FMA to compute (j + 0.5f) / n, ... (j + stride - 0.5) / n
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4244)
      auto py = _mm256_fmadd_ps(_mm256_set1_ps(j), pn_inv, pstep);
PDMATH_MSVC_WARNINGS_POP()
      // use mask to see if points are in unit circle + to increment count
      // note: treating as if everything is a single 8-bit int
      pn_in = _mm256_blendv_epi8(
        pn_in,                                               // false branch
        _mm256_add_epi32(pn_in, _mm256_set1_epi32(1)),       // true branch
        // note: since in_unit_circle is returning a float mask of 0xFFFFFFFF
        // and zeroes, we can just reinterpret it as an integral mask
        _mm256_castps_si256(detail::in_unit_circle(px, py))  // mask
      );
    }
    // remaining elements are added to the scalar sum
    // note: use float for all calculations
    // note: with optimization GCC complains about this being UB
    for (; j < n; j++)
      n_in += detail::in_unit_circle((i + 0.5f) / n, (j + 0.5f) / n);
  }
  // horizontally add lower and upper 128 bits
  // note: (x7 + x6, x5 + x4, x3 + x2, x1 + x0)
  auto pn_inh = _mm_hadd_epi32(
    _mm256_castsi256_si128(pn_in),      // low 128 bits (x3, x2, x1, x0)
    _mm256_extracti128_si256(pn_in, 1)  // high 128 bits (x7, x6, x5, x4)
  );
  // move upper 2 32-bit ints into lower 64 bits
  // note: (x7 + x6, x5 + x4, x7 + x6, x5 + x4)
  auto pn_inu = _mm_unpackhi_epi64(pn_inh, pn_inh);
  // perform an addition to futher accumulate
  // note: (2(x7 + x6), 2(x5 + x4), x3 + x2 + x7 + x6, x1 + x0 + x5 + x4)
  auto pn_sum1 = _mm_add_epi32(pn_inh, pn_inu);
  // swap lower (and highest) two 32-bit ints so we can get full summation
  // note: (2(x5 + x4), 2(x7 + x6), x1 + x0 + x5 + x4, x3 + x2 + x7 + x6)
  auto pn_sum2 = _mm_shuffle_epi32(pn_sum1, _MM_SHUFFLE(2, 3, 0, 1));
  // perform another addition and get lower 32-bit result to add to scalar sum
  // note: explicitly initialize as unsigned instead of signed
  n_in += _mm_cvtsi128_si32(_mm_add_epi32(pn_sum1, pn_sum2));
  // return pi estimate. we just cast to T
  // note: second static_cast<T> is unnecessary but is there to silence MSVC
  return 4 * (static_cast<T>(n_in) / static_cast<T>(n * n));
PDMATH_MSVC_WARNINGS_POP()
}
#endif  // PDMATH_HAS_AVX2

}  // namespace pdmath

#endif  // PDMATH_PI_H_
