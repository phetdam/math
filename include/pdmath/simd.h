/**
 * @file simd.h
 * @author Derek Huang
 * @brief C++ header for SIMD helpers
 * @copyright MIT License
 */

#ifndef PDMATH_SIMD_H_
#define PDMATH_SIMD_H_

#include <immintrin.h>

#include <cstddef>
#include <type_traits>
#include <utility>

#include "pdmath/features.h"
#include "pdmath/type_traits.h"
#include "pdmath/warnings.h"

namespace pdmath {

/**
 * Traits for a SIMD intrinsic type.
 *
 * For example, `__m256`, `__m512`, `__m128i`, etc.
 *
 * @tparam T type
 */
template <typename T>
struct is_simd_type : std::false_type {};

/**
 * Indicate if a type is a SIMD intrinsic type or not.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_simd_type_v = is_simd_type<T>::value;

/**
 * Macro defining a `is_simd_type` specialization.
 *
 * @tparam type SIMD type
 */
#define PDMATH_IS_SIMD_TYPE_DEF(type) \
  template <> \
  struct is_simd_type<type> : std::true_type {}

// GCC warns about attributes being ignored on template arguments
PDMATH_GNU_WARNINGS_PUSH()
PDMATH_GNU_WARNINGS_DISABLE("-Wignored-attributes")
// SSE specializations
#if PDMATH_HAS_SSE
PDMATH_IS_SIMD_TYPE_DEF(__m128);
PDMATH_IS_SIMD_TYPE_DEF(__m128d);
PDMATH_IS_SIMD_TYPE_DEF(__m128i);
#endif  // PDMATH_HAS_SSE

// AVX specialiations
#if PDMATH_HAS_AVX
PDMATH_IS_SIMD_TYPE_DEF(__m256);
PDMATH_IS_SIMD_TYPE_DEF(__m256d);
PDMATH_IS_SIMD_TYPE_DEF(__m256i);
#endif  // PDMATH_HAS_AVX

// AVX-512 specializations
#if PDMATH_HAS_AVX512F
PDMATH_IS_SIMD_TYPE_DEF(__m512);
PDMATH_IS_SIMD_TYPE_DEF(__m512d);
PDMATH_IS_SIMD_TYPE_DEF(__m512i);
#endif  // PDMATH_HAS_AVX512F

namespace detail {

/**
 * CPO for filling SIMD elements with increasing values.
 *
 * Each specialization is a regular callable type taking an initial value.
 *
 * @note We don't use a pack wrapper taking a function pointer to deduce the
 *  pack indexing required to remove the manual offsets done because intrinsic
 *  functions are *not* "real" C/C++ functions. Although GCC may implement them
 *  in a way that they can be treated as "normal" functions, MSVC will not
 *  recognize as intrinsics if used through a function pointer.
 *
 * @tparam T SIMD type
 * @tparam U Arithmetic type
 */
template <typename T, typename U>
struct iota_cpo {};

#if PDMATH_HAS_SSE
/**
 * Specialization for `__m128` and `float`.
 */
template <>
struct iota_cpo<__m128, float> {
  auto operator()(float v) const noexcept
  {
    return _mm_set_ps(v + 3, v + 2, v + 1, v);
  }
};
#endif  // PDMATH_HAS_SSE

#if PDMATH_HAS_SSE2
/**
 * Specialization for `__m128d` and `double`.
 */
template <>
struct iota_cpo<__m128d, double> {
  auto operator()(double v) const noexcept
  {
    return _mm_set_pd(v + 1, v);
  }
};

// TODO: add specialization for __m128i and char with _mm_set_epi8

/**
 * Specialization for `__m128i` and `short`.
 */
template <>
struct iota_cpo<__m128i, short> {
  auto operator()(short v) const noexcept
  {
    return _mm_set_epi16(v + 7, v + 6, v + 5, v + 4, v + 3, v + 2, v + 1, v);
  }
};

/**
 * Specialization for `__m128i` and `int`.
 */
template <>
struct iota_cpo<__m128i, int> {
  auto operator()(int v) const noexcept
  {
    return _mm_set_epi32(v + 3, v + 2, v + 1, v);
  }
};
#endif  // PDMATH_HAS_SSE2

#if PDMATH_HAS_AVX
/**
 * Specialization for `__m256` and `float`.
 */
template <>
struct iota_cpo<__m256, float> {
  auto operator()(float v) const noexcept
  {
    return _mm256_set_ps(v + 7, v + 6, v + 5, v + 4, v + 3, v + 2, v + 1, v);
  }
};

/**
 * Specialization for `__m256d` and `double`.
 */
template <>
struct iota_cpo<__m256d, double> {
  auto operator()(double v) const noexcept
  {
    return _mm256_set_pd(v + 3, v + 2, v + 1, v);
  }
};

/**
 * Specialization for `__m256i` and `int.
 */
template <>
struct iota_cpo<__m256i, int> {
  auto operator()(int v) const noexcept
  {
    return _mm256_set_epi32(v + 7, v + 6, v + 5, v + 4, v + 3, v + 2, v + 1, v);
  }
};
#endif  // PDMATH_HAS_AVX
PDMATH_GNU_WARNINGS_POP()

}  // namespace detail

/**
 * Fill the SIMD type elements with increasing values from the lowest address.
 *
 * This is essentially `std::iota` but operating on SIMD elements.
 *
 * @tparam T SIMD type
 * @tparam U Arithmetic type
 *
 * @param v Initial value
 */
template <typename T, typename U>
auto iota(
  U v = U{},
  constraint_t<std::is_arithmetic_v<U> || is_simd_type_v<U>> = 0) noexcept
{
  return detail::iota_cpo<T, U>{}(std::move(v));
}

}  // namespace pdmath

#endif  // PDMATH_SIMD_H_
