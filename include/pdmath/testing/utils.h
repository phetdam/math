/**
 * @file utils.h
 * @author Derek Huang
 * @brief C++ header for some testing utils
 * @copyright MIT License
 */

#ifndef PDMATH_TESTING_UTILS_H_
#define PDMATH_TESTING_UTILS_H_

#include <cassert>
#include <type_traits>

#include <gmock/gmock.h>

namespace pdmath {
namespace testing {

/**
 * Templated wrapper for a gradient + Hessian type wrapper.
 *
 * Specializations can be used with the Google Test `::testing::Types` template
 * for templated tests with `TYPED_TEST` in Google Test. Exposes the scalar
 * type as `scalar_type` and `value_type`, the gradient *Container* type as
 * `gradient_type`, and the Hessian type as `hessian_type`.
 *
 * @tparam V_t vector input type/gradient return type, a `*Container*
 * @tparam M_t Hessian return type, ex. an `Eigen::Matrix` specialization
 */
template <typename V_t, typename M_t>
class func_type_pair {
public:
  PDMATH_USING_FUNCTOR_TYPES(V_t, M_t);
  func_type_pair() = delete;
};

/**
 * Return a Google Test matcher for checking a *Container* is near zero.
 *
 * In other words, the returned matcher only allows `EXPECT_THAT` to succeed if
 * the max norm of the *Container* is less than or equal to `tol`.
 *
 * We can't use the Google Test `::testing::Each(::testing::FloatEq(0.))`
 * directly since we want to maintain the generic behavior of templates while
 * also being able to ask for different tolerance levels.
 *
 * @tparam T scalar type
 *
 * @param tol `const T&` all close tolerance
 */
template <typename T>
inline const auto match_all_near_zero(const T& tol)
{
  assert(tol >= 0);
  return ::testing::Each(
    ::testing::AllOf(::testing::Ge(-tol), ::testing::Le(tol))
  );
}

/**
 * Templated mixin class providing a static comparison tolerance function.
 *
 * Mostly just provides the float loose comparison tolerance we use a lot,
 * which is purposefully chosen to slightly smaller than the square root of the
 * corresponding `std::numeric_limits<T>::epsilon()` value.
 *
 * @tparam T scalar type
 */
template <typename T>
class tol_mixin {
public:
  /**
   * Return a floating-point comparison tolerance float-like types.
   *
   * @note `std::numeric_limits<double>::epsilon()` returns `2.22045e-16` while
   *     `std::numeric_limits<float>::epsilon()` returns `1.19209e-07`.
   */
// MSVC complains about how the return is unreachable in tol_mixin<float>. we
// disable C4702 (unreachable code) before the function, as warning pragmas
// for warning numbers >4699 used in a function only apply at function end.
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4702)
#endif  // _MSC_VER
  static T tol()
  {
    if constexpr (std::is_same_v<T, float>)
      return 1e-4f;
// MSVC complains about how this is truncated to float in tol_mixin<float>,
// which taken together with the C4702 warning about unreachable code makes for
// a bit of a nonsensical warning situation here.
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4305)
#endif  // _MSC_VER
    return 1e-8;
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
  }
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER

  /**
   * Return user-specified tolerance.
   */
  static T tol(const T& tol_override) { return tol_override; }

  // Google Test EXPECT_THAT matcher for checking that a *Container* is near
  // zero, using the default tolerance returned by tol().
  static inline const auto all_near_zero_matcher{match_all_near_zero(tol())};
};

}  // namespace testing
}  // namespace pdmath

#endif  // PDMATH_TESTING_UTILS_H_
