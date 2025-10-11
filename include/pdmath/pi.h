/**
 * @file pi.h
 * @author Derek Huabg
 * @brief C++ header for pi constants of varying precision.
 * @copyright MIT License
 */

#ifndef PDMATH_PI_H_
#define PDMATH_PI_H_

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
  static const auto abc = 1.3q;
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

}  // namespace pdmath

#endif  // PDMATH_PI_H_
