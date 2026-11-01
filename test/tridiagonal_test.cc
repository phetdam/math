/**
 * @file tridiagonal_test.cc
 * @author Derek Huang
 * @brief tridiagonal.h unit tests
 * @copyright MIT License
 */

#include "pdmath/tridiagonal.h"

#include <array>
#include <cstddef>
#include <limits>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/features.h"
#include "pdmath/testing/utils.h"
#include "pdmath/type_traits.h"
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
constexpr auto tridiagonal_view_init_test_inputs = std::make_tuple(
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
);

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
constexpr auto tridiagonal_solve_test_inputs = std::make_tuple(
  // system similar to cubic spline second derivative fitting
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
  // system similar to cubic spline second derivative fitting (float)

  std::tuple{
    [] { return std::array{0.3f, 0.2f, 0.6f, 0.7f, 0.f}; },
    [] { return std::array{2.f, 2.f, 2.f, 2.f, 2.f, 2.f}; },
    [] { return std::array{0.f, 0.7f, 0.8f, 0.4f, 0.3f}; },
    // note: this differs from the np.linalg.solve() result by slighly more
    // than 1 float eps. to some extent, this is not totally unsurprising, as
    // we are discarding precision and using Thomas's algorithm instead of LU
    // factorization, which is what np.linalg.solve() uses via LAPACK's gesv()
    []
    {
      return std::array{
        0.717f, 0.15644084f, 2.8885975f,
        -1.5943542f, 4.7163744f, 4.121f
      };
    },
    [] { return std::array{1.434f, 2.55f, 4.533f, 0.431f, 9.553f, 8.242f}; },
    // note: 2 float eps is enough to compare equal under single precision
    2 * std::numeric_limits<float>::epsilon()
  },
  // system similar to explicit method for heat equation
  std::tuple{
    [] { return std::array{1., 1., 1., 1., 1., 1.}; },
    [] { return std::array{-2., -2., -2., -2., -2., -2., -2.}; },
    [] { return std::array{1., 1., 1., 1., 1., 1.}; },
    []
    {
      return std::array{
        -3.2459625, -6.257925, -7.0498875, -7.50185,
        -6.530812500000001, -5.114575000000001, -2.5849375
      };
    },
    [] { return std::array{0.234, 2.22, 0.34, 1.423, 0.4452, 1.1134, 0.0553}; }
  },
  // system similar to Crank-Nicolson method for heat equation (float)
  std::tuple{
    [] { return std::array{-0.1f, -0.1f, -0.1f, -0.1f, -0.1f}; },
    [] { return std::array{2.115f, 2.2f, 2.2f, 2.2f, 2.2f, 2.115f}; },
    [] { return std::array{-0.1f, -0.1f, -0.1f, -0.1f, -0.1f}; },
    []
    {
      return std::array{
        0.64968175f, 0.7407692f, 1.323241f,
        2.8705335f, 0.72849727f, 0.8264065f
      };
    },
    [] { return std::array{1.3f, 1.4324f, 2.55f, 6.11f, 1.233f, 1.675f}; }
PDMATH_MSVC_WARNINGS_POP()
  }
);

/**
 * Partial specialization for `tridiagonal_solve_test<I>`.
 *
 * @tparam I Input index
 */
template <std::size_t I>
class TridiagonalTest<tridiagonal_solve_test<I>> : public ::testing::Test {
protected:
  static_assert(I < std::tuple_size_v<decltype(tridiagonal_solve_test_inputs)>);

  /**
   * Return the appropriate GoogleMock matcher for the type.
   *
   * @tparam T type
   */
  template <typename T>
  static auto matcher()
  {
    if constexpr (std::is_same_v<T, float>)
      return ::testing::FloatEq();
    else if constexpr (std::is_same_v<T, double>)
      return ::testing::DoubleEq();
    else
      return ::testing::Eq();
  }

  /**
   * Return the appropriate floating-point GoogleMock matcher.
   *
   * @tparam T type
   *
   * @param tol Maximum absolute tolerance
   */
  template <typename T>
  static auto matcher(
    pdmath::type_identity_t<T> tol,
    pdmath::constraint_t<std::is_floating_point_v<T>> = 0)
  {
    if constexpr (std::is_same_v<T, float>)
      return ::testing::FloatNear(tol);
    else
      return ::testing::DoubleNear(tol);
  }

// MSVC emits C4868 for tridiagonal_view ctor
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4868)
  /**
   * Solve the tridiagonal system and check that the results are as expected.
   */
  void operator()() const
  {
    // unpack input ranges
    auto input = std::get<I>(tridiagonal_solve_test_inputs);
    auto f1 = std::get<0>(input);
    auto f2 = std::get<1>(input);
    auto f3 = std::get<2>(input);
    auto f4 = std::get<3>(input);
    auto f5 = std::get<4>(input);
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
    // determine matcher
    auto mt = [&input]
    {
      using T = pdmath::range_value_t<decltype(xs)>;
      // explicit override
// MSVC emits C5258 erroneously as input still needs to be captured
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5258)
      if constexpr (5 < std::tuple_size_v<decltype(input)>)
        return matcher<T>(std::get<5>(input));
PDMATH_MSVC_WARNINGS_POP()
      // no override
      else
        return matcher<T>();
    }();
    // check results
    EXPECT_THAT(xs, ::testing::Pointwise(mt, x0));
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
