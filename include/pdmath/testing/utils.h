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

#include "pdmath/helpers.h"
#include "pdmath/type_traits.h"

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
 * @param tol All close tolerance
 */
template <typename T>
auto match_all_near_zero(T tol, constraint_t<std::is_floating_point_v<T>> = 0)
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
  static constexpr T tol() noexcept
  {
    if constexpr (std::is_same_v<T, float>)
      return 1e-4f;
    // note: without the else the compiler will always keep the second return
    // statement in. this correctly leads to C4702 and C4305 warnings with MSVC
    else
      return 1e-8;
  }

  /**
   * Return user-specified tolerance.
   */
  static constexpr T tol(T tol_override) noexcept { return tol_override; }

  // Google Test EXPECT_THAT matcher for checking that a *Container* is near
  // zero, using the default tolerance returned by tol().
  static inline const auto all_near_zero_matcher{match_all_near_zero(tol())};
};

/**
 * Templated wrapper for holding a type and a value of appropriate type.
 *
 * @tparam T type
 * @tparam v `auto` value, e.g. integral type, pointer, lvalue reference
 */
template <typename T, auto v>
class type_value_pair {
public:
  using type = T;
  using value_type = decltype(v);
  type_value_pair() = delete;
  static constexpr value_type value = v;
};

/**
 * Traits type test case template base for Google Test templated tests.
 *
 * This takes a unary type traits and a `type_value_pair`.
 *
 * @tparam Traits *UnaryTypeTrait* class template
 * @tparam T `type_value_pair` specialization
 */
template <template <typename...> typename Traits, typename T>
class traits_test {};

/**
 * Partial specialization for `type_value_pair`.
 *
 * @tparam Traits *UnaryTypeTrait* class template
 * @tparam T Input type
 * @tparam v Expected truth result
 */
template <template <typename...> typename Traits, typename T, bool v>
class traits_test<Traits, type_value_pair<T, v>> : public ::testing::Test {
protected:
  // truth result of evaluating the traits
  static constexpr bool value_ = Traits<T>::value;
  // expected truth result
  static constexpr bool expected_ = v;
  // operator() for triggering EXPECT_EQ
  constexpr void operator()() const noexcept { EXPECT_EQ(expected_, value_); }
};

/**
 * `TYPED_TEST` default implementation for `traits_test`.
 *
 * This wraps `TYPED_TEST` and removes the need to define the test body. The
 * test name used for `TYPED_TEST` is `TraitsValueTest`.
 *
 * @param fixture `traits_test` test fixture type name
 */
#define PDMATH_TRAITS_TEST(fixture) \
  /* static_assert used to enforce semicolon usage */ \
  TYPED_TEST(fixture, TraitsValueTest) { (*this)(); } static_assert(true)

}  // namespace testing
}  // namespace pdmath

#endif  // PDMATH_TESTING_UTILS_H_
