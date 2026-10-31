/**
 * @file utils.h
 * @author Derek Huang
 * @brief C++ header for some testing utils
 * @copyright MIT License
 */

#ifndef PDMATH_TESTING_UTILS_H_
#define PDMATH_TESTING_UTILS_H_

#include <cassert>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

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
   *  `std::numeric_limits<float>::epsilon()` returns `1.19209e-07`.
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

  /**
   * Return GMock matcher checking that all container values are near zero.
   *
   * An optional tolerance is provided with the default from `tol()`.
   *
   * @param t Tolerance value
   */
  auto AllNearZero(T t = tol()) const
  {
    return match_all_near_zero(t);
  }
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
 * Templated wrapper for holding a type as its only member.
 *
 * This is useful for embedding types in a `std::tuple` in situations when one
 * or more of the types may be incomplete or have unknown extent.
 *
 * @tparam T type
 */
template <typename T>
struct type_wrapper {
  using type = T;
};

/**
 * Templated wrapper for holding two types as its only members.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U>
struct type_pair_wrapper {
  using first_type = T;
  using second_type = U;
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
 * This is used for testing traits types that have a `constexpr` non-type
 * member `value` we expect to equal a certain value.
 *
 * @tparam Traits *UnaryTypeTrait* class template
 * @tparam T Input type
 * @tparam v Expected result value
 */
template <template <typename...> typename Traits, typename T, auto v>
class traits_test<Traits, type_value_pair<T, v>> : public ::testing::Test {
protected:
  // operator() for triggering EXPECT_EQ
  void operator()() const
  {
    EXPECT_EQ(Traits<T>::value, v);
  }
};

namespace detail {

/**
 * Traits type to indicate if a type has a type member `type`.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct has_type_member : std::false_type {};

/**
 * True specialization for types with the type member `type`.
 *
 * @tparam T type
 */
template <typename T>
struct has_type_member<T, std::void_t<typename T::type> > : std::true_type {};

}  // namespace detail

/**
 * Partial specialization for `type_pair_wrapper<T, U>`.
 *
 * This is used for testing traits types that have a type member `type` which
 * we expect will be the same type as the `U` of the `type_pair_wrapper<T, U>`.
 *
 * @tparam Traits Traits type expected to have a `type` member
 * @tparam T Input type
 * @tparam U Expected type
 */
template <template <typename...> typename Traits, typename T, typename U>
class traits_test<Traits, type_pair_wrapper<T, U>> : public ::testing::Test {
protected:
  // operator() for triggering EXPECT_EQ
  void operator()() const
  {
    // if Traits<T> has no member type this is a separate error
    if constexpr (!detail::has_type_member<Traits<T>>::value)
      GTEST_FAIL() << "traits type missing type member";
    // otherwise, check that types are the same
    else
      EXPECT_TRUE((std::is_same_v<typename Traits<T>::type, U>));
  }
};

/**
 * Partial specialization for `type_pair_wrapper<T, always_false<U>>`.
 *
 * This is used for testing traits type that have a type member `type` which
 * we expect will *not* be the same as the `U` of the
 * `type_pair_wrapper<T, always_false<U>>`.
 *
 * @tparam Traits Traits type expected to have a `type` member
 * @tparam T Input type
 * @tparam U Expected type
 */
template <template <typename...> typename Traits, typename T, typename U>
class traits_test<Traits, type_pair_wrapper<T, always_false<U>> >
  : public ::testing::Test {
protected:
    // operator() for triggering EXPECT_EQ
    void operator()() const
    {
      // if Traits<T> has no member type this is a separate error
      if constexpr (!detail::has_type_member<Traits<T>>::value)
        GTEST_FAIL() << "traits type missing type member";
      // otherwise, check that types are *not* the same
      else
        EXPECT_FALSE((std::is_same_v<typename Traits<T>::type, U>));
    }
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

// TODO: consider if a macro like the following would help reduce boilerplate
#if 0
#define PDMATH_DEFINE_TRAITS_TEST(test, traits, ...) \
  template <typename T> \
  class test : public pdmath::testing::traits_test<traits, T> {}; \
  using PDMATH_CONCAT(test, Types) = ::testing::Types<__VA_ARGS__>; \
  TYPED_TEST_SUITE(test, PDMATH_CONCAT(test, Types)); \
  PDMATH_TRAITS_TEST(test)
#endif  // 0

/**
 * Template type index.
 *
 * This is used as an alternative to `std::integral_constant` or a single-
 * element version of `std::index_sequence` for brevity in CTest output. The
 * index value can be accessed via `value` or via the constexpr `operator()`.
 *
 * @tparam I Index value
 */
template <std::size_t I>
struct index {
  static constexpr auto value = I;
  constexpr auto operator()() const noexcept { return I; }
};

/**
 * Construct `::testing::Types<...>` from the given type.
 *
 * For example, `T` can be a `std::index_sequence<Is...>`, with an appropriate
 * partial specialization for `::testing::Types<index<Is>...>` as the type.
 *
 * @tparam T type
 */
template <typename T>
struct test_types {};

/**
 * SFINAE helper to get the `::testing::Types<...>` from `test_types<T>`.
 *
 * @tparam T type
 */
template <typename T>
using test_types_t = typename test_types<T>::type;

/**
 * Partial specialization for a `std::index_sequence<Is...>`.
 *
 * This is useful when instantiating each GoogleTest test template with
 * `index<Is>` for indexing into an array or tuple of test inputs.
 *
 * @tparam Is Indices 0 through the size of the input range - 1
 */
template <std::size_t... Is>
struct test_types<std::index_sequence<Is...>> {
  using type = ::testing::Types<index<Is>...>;
};

/**
 * SFINAE helper for the `::testing::Types<index<Is>...>` from a size.
 *
 * @tparam N Number of test inputs
 */
template <std::size_t N>
using index_types = test_types_t<std::make_index_sequence<N>>;

/**
 * Traits type for `::testing::Types<index<Is>...>` from sized types.
 *
 * This is for ranges that have compile-time sizing or tuples.
 *
 * @tparam T type
 */
template <typename T>
struct index_types_from {};

/**
 * SFINAE helper for `::testing::Types<index<Ts>...>` from sized types.
 *
 * @tparam T type
 */
template <typename T>
using index_types_from_t = typename index_types_from<std::decay_t<T>>::type;

/**
 * Partial specialization for `std::tuple<...>`.
 *
 * @tparam Ts types
 */
template <typename... Ts>
struct index_types_from<std::tuple<Ts...>>
  : test_types<std::index_sequence_for<Ts...>> {};


/**
 * Partial specialization to support `::testing::Types<T<Is>...>`.
 *
 * This allows use of a custom template template that takes a `std::size_t`
 * like `index<I>` which is often used to distinguish groups of tests. Each
 * group would correspond to a different test input container and would be
 * expressed via partial specializations of the test class template.
 *
 * @note Usually one will never need to use this directly.
 *
 * @tparam T Template template type
 * @tparam Is Indices 0 through the size of the input range - 1
 */
template <template <std::size_t> typename T, std::size_t... Is>
struct test_types<type_pair_wrapper<T<0>, std::index_sequence<Is...>>> {
  using type = ::testing::Types<T<Is>...>;
};

/**
 * SFINAE helper for the `::testing::Types<T<Is>...>` from a size and template.
 *
 * @tparam T Template template taking the test input index
 * @tparam N Number of test inputs
 */
template <template <std::size_t> typename T, std::size_t N>
using indexed_types =
  typename test_types<type_pair_wrapper<T<0>, std::make_index_sequence<N>>>::type;

}  // namespace testing
}  // namespace pdmath

#endif  // PDMATH_TESTING_UTILS_H_
