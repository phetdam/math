/**
 * @file span_test.cc
 * @author Derek Huang
 * @brief span.h unit tests
 * @copyright MIT License
 */

#include "pdmath/span.h"

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/common.h"
#include "pdmath/testing/utils.h"
#include "pdmath/warnings.h"

namespace {

/**
 * `span` template test fixture class.
 *
 * @tparam T type
 */
template <typename T>
class SpanTest : public ::testing::Test {};

/**
 * Test case type for `span` initialization tests.
 *
 * @tparam I Input index
 */
template <std::size_t I>
struct span_init_test {};

// span_init_test input cases. each is a std::pair containing a nullary
// callable to produce a host range and a unary callable taking the host range
// as an lvalue for span initialization, which can be arbitrary
constexpr auto span_init_test_inputs = std::tuple{
  // pointer + size init
  std::pair{
    [] { return std::vector{1, 2, 3, 4, 5}; },
    [](auto& v) { return pdmath::span{v.data(), v.size()}; }
  },
  // init from iterator + size
  std::pair{
    [] { return std::vector{2., 4., 6., 8.}; },
    [](auto& v) { return pdmath::span{v.begin(), v.size()}; }
  },
  // init from start + end iterators
  std::pair{
    [] { return std::vector{1u, 2u, 3u, 4u, 5u}; },
    [](auto& v) { return pdmath::span{v.begin(), v.end()}; }
  },
  // init from static array's raw iterators
  std::pair{
    []() -> auto&
    {
      static constexpr int v[] = {2, 4, 6, 8};
      return v;
    },
    [](auto& v) { return pdmath::span{std::begin(v), std::end(v)}; }
  }
};

/**
 * Partial specialization for `span_init_test<I>`.
 *
 * @tparam I Input index
 */
template <std::size_t I>
class SpanTest<span_init_test<I>> : public ::testing::Test {
protected:
  static_assert(I < std::tuple_size_v<decltype(span_init_test_inputs)>);

  /**
   * Check that the initialized span has the correct referenced type + values.
   */
  void operator()() const
  {
    // get invocables
    auto [make_v, make_s] = std::get<I>(span_init_test_inputs);
    // create range + span
    // note: need decltype(auto) to ensure prvalues are values, not references
    decltype(auto) v = make_v();
    auto s = make_s(v);
    // check referenced sequence type
    EXPECT_TRUE(
      (
        std::is_same_v<
          pdmath::range_reference_t<decltype(v)>,
          pdmath::range_reference_t<decltype(s)>
        >
      )
    );
    // check values
    EXPECT_THAT(s, ::testing::Pointwise(::testing::Eq(), v));
  }
};

/**
 * Test case type for `span` traits tests.
 *
 * @tparam T Traits type
 */
template <typename T>
struct span_traits_test {};

/**
 * Partial specialization for `span_traits_test<Traits<Ts...>>`.
 *
 * @tparam Traits Traits type template
 * @tparam Ts Traits type parameters
 */
template <template <typename...> typename Traits, typename... Ts>
class SpanTest<span_traits_test<Traits<Ts...>> > : public ::testing::Test {
protected:
  /**
   * Check that `Traits<Ts...>::value` evaluates to `true`.
   */
  void operator()() const
  {
    EXPECT_TRUE(Traits<Ts...>::value);
  }
};

/**
 * Partial specialization for `span_traits_test<always_false<Traits<Ts...>>>`.
 *
 * @tparam Traits Traits type template
 * @tparam Ts Traits type parameters
 */
template <template <typename...> typename Traits, typename... Ts>
class SpanTest<span_traits_test<pdmath::always_false<Traits<Ts...>>> >
  : public ::testing::Test {
protected:
  /**
   * Check that `Traits<Ts...>::value` evaluates to `false`.
   */
  void operator()() const
  {
    EXPECT_FALSE(Traits<Ts...>::value);
  }
};

/**
 * Test case type for `span` range transform tests.
 *
 * @tparam I Input index
 */
template <std::size_t I>
struct span_transform_test {};

// span_transform_test input cases. each is a std::tuple containing a nullary
// callable to produce a host range, a unary callable taking the host range
// for modification via span iteration, and another unary callable producing
// the host range with the values one is expected to see after modification
constexpr auto span_transform_test_inputs = std::tuple{
  // range-for
  std::tuple{
    [] { return std::vector{9, 8, 7, 6, 5, 4}; },
    [](auto& v)
    {
      pdmath::span s{v.begin(), v.end()};
      for (auto& x : s)
        x--;
    },
    [] { return std::vector{8, 7, 6, 5, 4, 3}; }
  },
  // operator[] iteration
// note: MSVC emits C5246 in C++17 mode due to old interpretation of std::array
// of aggregate without brace elision; see the CWG defect report:
// https://www.open-std.org/JTC1/SC22/WG21/docs/papers/2012/n3367.html#1270
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5246)
  std::tuple{
    [] { return std::array{2., 4., 6., 8.}; },
    [](auto& v)
    {
      pdmath::span s{v.begin(), v.end()};
      for (auto i = 0u; i < s.size(); i++)
        s[i] /= 2;
    },
    [] { return std::array{1., 2., 3., 4.}; }
PDMATH_MSVC_WARNINGS_POP()
  },
  // iterators
  std::tuple{
    [] { return std::vector{1u, 2u, 3u, 4u, 5u}; },
    [](auto& v)
    {
      pdmath::span s{v.data(), v.size()};
      auto it = s.begin();
      while (it != s.end())
        *it++ +=3u;
    },
    [] { return std::vector{4u, 5u, 6u, 7u, 8u}; }
  }
};

/**
 * Partial specialization for `span_transform_test<I>`.
 *
 * @tparam I Input index
 */
template <std::size_t I>
class SpanTest<span_transform_test<I>> : public ::testing::Test {
protected:
  static_assert(I < std::tuple_size_v<decltype(span_transform_test_inputs)>);

  /**
   * Check that modification of the range through a span works as expected.
   */
  void operator()() const
  {
    // get invocables
    auto [input, transform, target] = std::get<I>(span_transform_test_inputs);
    // create input, transform, and compare
    auto v0 = input();
    transform(v0);
    EXPECT_THAT(v0, ::testing::Pointwise(::testing::Eq(), target()));
  }
};

// SpanTest input types
TYPED_TEST_SUITE(
  SpanTest,
  PDMATH_IDENTITY(
    pdmath::testing::merge_test_types_t<
      // span_init_test
      pdmath::testing::indexed_types<
        span_init_test,
        std::tuple_size_v<decltype(span_init_test_inputs)>
      >,
      // span_traits_test
      pdmath::testing::projected_types<
        span_traits_test,
        std::is_trivially_copyable<pdmath::span<double>>,
        std::is_move_constructible<pdmath::span<int>>,
        std::is_copy_constructible<pdmath::span<double>>,
        std::is_default_constructible<pdmath::span<const int>>,
        std::is_move_assignable<pdmath::span<float>>,
        std::is_copy_assignable<pdmath::span<std::string>>,
        std::is_constructible<pdmath::span<const double>, pdmath::span<double>>,
        pdmath::always_false<
          std::is_constructible<pdmath::span<double>, pdmath::span<const double>>
        >,
        pdmath::always_false<
          std::is_constructible<pdmath::span<int>, pdmath::span<double>>
        >,
        std::is_constructible<pdmath::span<const double>, double*, std::size_t>,
        pdmath::always_false<
          std::is_constructible<pdmath::span<double>, void*, void*>
        >,
        std::is_constructible<pdmath::span<const int*>, int*, int*>
      >,
      // span_transform_test
      pdmath::testing::indexed_types<
        span_transform_test,
        std::tuple_size_v<decltype(span_transform_test_inputs)>
      >
    >
  )
);

/**
 * Execute the test.
 */
TYPED_TEST(SpanTest, Test)
{
  (*this)();
}

}  // namespace
