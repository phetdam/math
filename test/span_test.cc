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

#include <gtest/gtest.h>

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
   * Check that the initialized span has the correct value type + size.
   */
  void operator()() const
  {
    // get invocables
    auto [make_v, make_s] = std::get<I>(span_init_test_inputs);
    // create range + span
    // note: need decltype(auto) to ensure prvalues are values, not references
    decltype(auto) v = make_v();
    auto s = make_s(v);
    // check type
    EXPECT_TRUE(
      (
        std::is_same_v<
          pdmath::range_reference_t<decltype(v)>,
          pdmath::range_reference_t<decltype(s)>
        >
      )
    );
    // check size
    EXPECT_EQ(std::size(v), std::size(s));
  }
};

/**
 * Traits type to create a `::testing::Types<...>` for `span_init_test`.
 *
 * @tparam T type
 */
template <typename T>
struct span_init_test_types {};

/**
 * Partial specialization for a `std::index_sequence<Is...>`.
 *
 * @tparam Is Indices 0 through size of `span_init_test_inputs` - 1
 */
template <std::size_t... Is>
struct span_init_test_types<std::index_sequence<Is...>> {
  using type = ::testing::Types<span_init_test<Is>...>;
};

/**
 * `void` specialization using the size of `span_init_test_inputs`.
 */
template <>
struct span_init_test_types<void> : span_init_test_types<
  std::make_index_sequence<std::tuple_size_v<decltype(span_init_test_inputs)>> >
  {};

/**
 * Type alias for the `::testing::Types<span_init_test<Is>...>` types.
 */
using span_init_test_types_t = typename span_init_test_types<void>::type;

TYPED_TEST_SUITE(SpanTest, span_init_test_types_t);

/**
 * Execute the test.
 */
TYPED_TEST(SpanTest, Test)
{
  (*this)();
}

}  // namespace
