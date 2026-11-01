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

#include "pdmath/features.h"
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
    [] { return std::array{1., 1., 1., 1., 0.}; },
    [] { return std::array{2., 2., 2., 2., 2., 2.}; },
    [] { return std::array{0., 1., 1., 1., 1.}; }
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

/**
 * Test case for `tridiagonal_view` stream tests.
 *
 * @tparam I Input index
 */
template <std::size_t I>
struct tridiagonal_view_stream_test {};

// tridiagonal_view_stream_test input cases. each is a std::tuple with nullary
// callables to produce the lower, main, upper diagonal ranges, as well as a
// string literal indicating the expected stream output
constexpr auto tridiagonal_view_stream_test_inputs = std::tuple{
  std::tuple{
    [] { return std::vector{0.5, 0.4, 0.4, 0.6, 0.}; },
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5246)
    [] { return std::array{2., 2., 2., 2., 2., 2.}; },
PDMATH_MSVC_WARNINGS_POP()
    [] { return std::vector{0., 0.5, 0.6, 0.6, 0.4}; },
    "  2    0    0    0    0    0\n"
    "0.5    2  0.5    0    0    0\n"
    "  0  0.4    2  0.6    0    0\n"
    "  0    0  0.4    2  0.6    0\n"
    "  0    0    0  0.6    2  0.4\n"
    "  0    0    0    0    0    2"
  },
  std::tuple{
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5246)
    [] { return std::array{1.4, 1.352, 2.432, 0.223}; },
    [] { return std::vector{4., 5., 5., 6., 5.}; },
    [] { return std::array{1.2, 3.2, 1.111, 1.453}; },
PDMATH_MSVC_WARNINGS_POP()
    "  4    1.2      0      0      0\n"
    "1.4      5    3.2      0      0\n"
    "  0  1.352      5  1.111      0\n"
    "  0      0  2.432      6  1.453\n"
    "  0      0      0  0.223      5"
  }
};
// helper for inputs size since variable name is long
constexpr auto tridiagonal_view_stream_test_inputs_size =
  std::tuple_size_v<decltype(tridiagonal_view_stream_test_inputs)>;

/**
 * Partial specialization for `tridiagonal_view_stream_test<I>`.
 *
 * @tparam I Input index
 */
template <std::size_t I>
class TridiagonalTest<tridiagonal_view_stream_test<I>> : public ::testing::Test {
protected:
  static_assert(I < tridiagonal_view_stream_test_inputs_size);

// MSVC emits C4868 for tridiagonal_view ctor
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4868)
  /**
   * Create the `tridiagonal_view` and ensure it formats as expected.
   */
  void operator()() const
  {
    // get input invocables and expected result
    auto [f1, f2, f3, str] = std::get<I>(tridiagonal_view_stream_test_inputs);
    // create input ranges + view
    auto ls = f1();
    auto ms = f2();
    auto us = f3();
    pdmath::tridiagonal_view tri{ls, ms, us};
PDMATH_MSVC_WARNINGS_POP()
    // format + check result
    std::stringstream ss;
    ss << tri;
    // note: in C++20 we can use view()
#if PDMATH_HAS_CXX20
    EXPECT_EQ(str, ss.view());
#else
    EXPECT_EQ(str, ss.str());
#endif  // !PDMATH_HAS_CXX20
  }
};

/**
 * Test case for `tridiagonal_view` linear system solution tests.
 *
 * @tparam I Input index
 */
template <std::size_t I>
struct tridiagonal_solve_test {};

// tridiagonal_solve_test inputs. each is a std::tuple with nullary invocables
// for the lower, main, and upper diagonal ranges to construct from, the
// expected solution, and the right-hand-side coefficient range invocable. all
// values are computed using NumPy's numpy.linalg.solve() for comparison
constexpr auto tridiagonal_solve_test_inputs = std::tuple{
  // system similar to cubic spline fitting
  std::tuple{
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5246)
    [] { return std::array{0.4, 0.5, 0.5, 0.6, 0.}; },
    [] { return std::array{2., 2., 2., 2., 2., 2.}; },
    [] { return std::array{0., 0.6, 0.5, 0.5, 0.4}; },
    []
    {
      return std::array{
        0.65, -0.30581560283687936, 1.3193853427895978,
        -0.33172576832151296, 0.4475177304964539, 1.61
      };
    },
    [] { return std::array{1.3, 0.44, 2.32, 0.22, 1.34, 3.22}; }
  },
  // system similar to cubic spline fitting
  std::tuple{
    [] { return std::array{0.3, 0.2, 0.6, 0.7, 0.}; },
    [] { return std::array{2., 2., 2., 2., 2., 2.}; },
    [] { return std::array{0., 0.7, 0.8, 0.4, 0.3}; },
    []
    {
      return std::array{
        0.717, 0.15644086436426763, 2.888597530387807,
        -1.5943540420605828, 4.716373914721204, 4.121
      };
    },
    [] { return std::array{1.434, 2.55 , 4.533, 0.431, 9.553, 8.242}; }
PDMATH_MSVC_WARNINGS_POP()
  }
};

/**
 * Partial specialization for `tridiagonal_solve_test<I>`.
 *
 * @tparam I Input index
 */
template <std::size_t I>
class TridiagonalTest<tridiagonal_solve_test<I>> : public ::testing::Test {
protected:
  static_assert(I < std::tuple_size_v<decltype(tridiagonal_solve_test_inputs)>);

// MSVC emits C4868 for tridiagonal_view ctor
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4868)
  /**
   * Solve the tridiagonal system and check that the results are as expected.
   */
  void operator()() const
  {
    // inputs
    auto [f1, f2, f3, f4, f5] = std::get<I>(tridiagonal_solve_test_inputs);
    // create view
    auto ls = f1();
    auto ms = f2();
    auto us = f3();
    pdmath::tridiagonal_view tri{ls, ms, us};
PDMATH_MSVC_WARNINGS_POP()
    // expected result + coefficient vector
    auto x0 = f4();
    auto rhs = f5();
    // solve system
    // note: for template type deduction convenience we just copy from x0
    std::vector xs(std::begin(x0), std::end(x0));
    tri(xs, rhs);
    // check results
    EXPECT_THAT(xs, ::testing::Pointwise(::testing::DoubleEq(), x0));
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
      >,
      pdmath::testing::indexed_types<
        tridiagonal_view_stream_test,
        std::tuple_size_v<decltype(tridiagonal_view_stream_test_inputs)>
      >,
      pdmath::testing::indexed_types<
        tridiagonal_solve_test,
        std::tuple_size_v<decltype(tridiagonal_solve_test_inputs)>
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
