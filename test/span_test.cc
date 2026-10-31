/**
 * @file span_test.cc
 * @author Derek Huang
 * @brief span.h unit tests
 * @copyright MIT License
 */

#include "pdmath/span.h"

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/common.h"
#include "pdmath/testing/utils.h"

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
 * Partial specialization for `span_init_test<T>`.
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
 * The unary type traits will be applied to only a `span<double>` as currently
 * there is no particular specialization or optimization for a value type.
 *
 * @tparam Traits Traits type template
 */
template <template <typename> typename Traits>
struct span_traits_test {};

/**
 * Partial specialization for `span_traits_test<Traits>`.
 *
 * @tparam Traits Traits type template
 */
template <template <typename> typename Traits>
class SpanTest<span_traits_test<Traits>> : public ::testing::Test {
protected:
  /**
   * Check that `Traits<span<double>>::value` evaluates to `true`.
   */
  void operator()() const
  {
    EXPECT_TRUE(Traits<pdmath::span<double>>::value);
  }
};

// SpanTest input types
TYPED_TEST_SUITE(
  SpanTest,
  PDMATH_IDENTITY(
    pdmath::testing::merge_test_types_t<
      pdmath::testing::indexed_types<
        span_init_test,
        std::tuple_size_v<decltype(span_init_test_inputs)>
      >,
      ::testing::Types<
        span_traits_test<std::is_trivially_copyable>,
        span_traits_test<std::is_move_constructible>,
        span_traits_test<std::is_copy_constructible>,
        span_traits_test<std::is_default_constructible>,
        span_traits_test<std::is_move_assignable>,
        span_traits_test<std::is_copy_assignable>
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
