/**
 * @file tridiagonal_test.cc
 * @author Derek Huang
 * @brief tridiagonal.h unit tests
 * @copyright MIT License
 */

#include "pdmath/tridiagonal.h"

#include <array>
#include <cstddef>
#include <sstream>
#include <tuple>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/testing/utils.h"
#include "pdmath/warnings.h"

namespace {

/**
 * Tridiagonal matrix view and system solution test fixture template.
 *
 * @tparam T type
 */
template <typename T>
class TridiagonalTest : public ::testing::Test {};

/**
 * Test case for `tridiagonal_view` initialization tests.
 *
 * @tparam I Input index
 */
template <std::size_t I>
struct tridiagonal_view_init_test {};

// tridiagonal_view_init_test input cases. each is a std::tuple that has a bool
// to indicate if view construction should be evaluated at compile time, and
// nullary invocables for each of the diagonals
constexpr auto tridiagonal_view_init_test_inputs = std::tuple{
  // C arrays
  // note: static constexpr does *not* enable constexpr evaluation
  std::tuple{
    false,
    []() -> auto&
    {
      static float ls[] = {1.f, 1.f, 1.f, 1.f};
      return ls;
    },
    []() -> auto&
    {
      static float ms[] = {2.f, 2.f, 2.f, 2.f, 2.f};
      return ms;
    },
    []() -> auto&
    {
      static float us[] = {1.f, 1.f, 1.f, 1.f};
      return us;
    }
  },
  // std::array
  std::tuple{
    false,
// note: MSVC emits C5246 in C++17 mode due to old interpretation of std::array
// of aggregate without brace elision; see the CWG defect report:
// https://www.open-std.org/JTC1/SC22/WG21/docs/papers/2012/n3367.html#1270
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5246)
    [] { return std::array{1., 1., 1., 1., 1.}; },
    [] { return std::array{2., 2., 2., 2., 2., 2.}; },
    [] { return std::array{1., 1., 1., 1., 1.}; }
PDMATH_MSVC_WARNINGS_POP()
  },
  // std::vector
  std::tuple{
    false,
    [] { return std::vector{1., 1., 1., 1.}; },
    [] { return std::vector{2., 2., 2., 2., 2.}; },
    [] { return std::vector{0.8, 0.8, 0.8, 0.8}; }
  }
};

/**
 * Partial specialization for `tridiagonal_view_init_test<I>`.
 *
 * @tparam I Input index
 */
template <std::size_t I>
class TridiagonalTest<tridiagonal_view_init_test<I>> : public ::testing::Test {
protected:
  static_assert(I < std::tuple_size_v<decltype(tridiagonal_view_init_test_inputs)>);

// MSVC emits C4868 for the non-constexpr tridiagonal_view ctor
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4868)
  /**
   * Ensure that the `tridiagonal_view` init is as expected.
   *
   * The dimensions and values of the `tridiagonal_view` are checked.
   */
  void operator()() const
  {
    // get inputs
    // note: structured binding is only constexpr in C++20
    constexpr auto input = std::get<I>(tridiagonal_view_init_test_inputs);
    constexpr auto compile_time = std::get<0>(input);
    constexpr auto make_ld = std::get<1>(input);
    constexpr auto make_md = std::get<2>(input);
    constexpr auto make_ud = std::get<3>(input);
    // compile-time construction
    if constexpr (compile_time) {
      // create tridiagonal view
      // note: use decltype(auto) to preserve reference category
      constexpr decltype(auto) ls = make_ld();
      constexpr decltype(auto) ms = make_md();
      constexpr decltype(auto) us = make_ud();
      constexpr pdmath::tridiagonal_view tri{ls, ms, us};
      // check dimensions and values
      EXPECT_EQ(std::size(ms), tri.rows());
      EXPECT_EQ(std::size(ms), tri.cols());
      EXPECT_THAT(tri.ld(), ::testing::Pointwise(::testing::Eq(), ls));
      EXPECT_THAT(tri.md(), ::testing::Pointwise(::testing::Eq(), ms));
      EXPECT_THAT(tri.ud(), ::testing::Pointwise(::testing::Eq(), us));
    }
    // run-time construction
    else {
      // create tridiagonal view
      // note: use decltype(auto) to preserve reference category
      decltype(auto) ls = make_ld();
      decltype(auto) ms = make_md();
      decltype(auto) us = make_ud();
      pdmath::tridiagonal_view tri{ls, ms, us};
PDMATH_MSVC_WARNINGS_POP()
      // check dimensions and values
      EXPECT_EQ(std::size(ms), tri.rows());
      EXPECT_EQ(std::size(ms), tri.cols());
      EXPECT_THAT(tri.ld(), ::testing::Pointwise(::testing::Eq(), ls));
      EXPECT_THAT(tri.md(), ::testing::Pointwise(::testing::Eq(), ms));
      EXPECT_THAT(tri.ud(), ::testing::Pointwise(::testing::Eq(), us));
    }
  }
};

// TridiagonalTest input types
TYPED_TEST_SUITE(
  TridiagonalTest,
  PDMATH_IDENTITY(
    pdmath::testing::merge_test_types_t<
      pdmath::testing::indexed_types<
        tridiagonal_view_init_test,
        std::tuple_size_v<decltype(tridiagonal_view_init_test_inputs)>
      >
    >
  )
);

/**
 * Evaluate the test case.
 */
TYPED_TEST(TridiagonalTest, Test)
{
  (*this)();
}

}  // namespace
