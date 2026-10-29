/**
 * @file akima_spline_test.cc
 * @author Derek Huang
 * @brief akima_spline.h unit tests
 * @copyright MIT License
 */

#include "pdmath/akima_spline.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/common.h"
#include "pdmath/testing/utils.h"
#include "pdmath/type_traits.h"
#include "pdmath/warnings.h"

namespace {

/**
 * Akima spline template test case fixture.
 *
 * @tparam T Input type
 */
template <typename T = void>
class AkimaSplineTest : public ::testing::Test {};

/**
 * Base specialization for the `AkimaSplineTest`.
 *
 * This is used to hold some shared static members such as spline creation
 * functions, etc., that could be shared by multiple test cases.
 */
template <>
class AkimaSplineTest<void> : public ::testing::Test {
public:
  /**
   * Return an Akima spline over 5 knot points.
   *
   * This was the original and first test spline used.
   */
  static auto spline_1()
  {
    return pdmath::akima_spline{
      {1., 2., 3., 4., 5.},
      {2., 5., 6., 4., 5.}
    };
  }

  /**
   * Return an Akima spline over 7 knot points.
   *
   * This is a slightly more involved testing spline.
   */
  static auto spline_2()
  {
    // note: initializer lists need not have floating-point value types
    return pdmath::akima_spline{
      {-3, -2, -1, 0, 1, 2, 3},
      {1.5, 4.5, -1., -2., 0.2, 3.4, 2.3}
    };
  }

  /**
   * Return an Akima spline over the MATLAB example piecewise polynomial.
   *
   * Knot data is from https://www.mathworks.com/help/matlab/ref/makima.html.
   */
  static auto spline_3()
  {
    // note: initializer lists need not have floating-point value types
    return pdmath::akima_spline{
      {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5},
      {1., 1., 1., 0., 0., 1., 1., 2., 2., 2., 2.}
    };
  }
};

// note: MSVC emits C5246 in C++17 mode due to old interpretation of std::array
// of aggregate without brace elision; see the CWG defect report:
// https://www.open-std.org/JTC1/SC22/WG21/docs/papers/2012/n3367.html#1270
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5246)
// Akima spline test case inputs. each input is a std::tuple with the elements:
//
// 1. nullary invocable returning an akima_spline (flexible initialization)
// 2. std::array evaluation points (note: because std::array is constexpr)
// 3. result specification std::tuple<T0, T1, T2, T3, T4>
//
// each Tk in the result std::tuple corresponds to the result for the kth
// derivative of the spline, i.e. T0 are the spline values, T1 are the spline
// first derivative values, etc., and Tk can be either a std::array of the same
// size as the evaluation points, or a std::pair or said std::array and a
// pdmath::testing::type_wrapper<T> instance that is used to tell GoogleTest
// whether ::testing::DoubleEq() or ::testing::FloatEq() should be used in the
// GoogleMock matcher. the GoogleMock matchers are not constexpr so we cannot
// use them directly; the type_wrapper<T> provides the needed type information.
//
// all expected values were computed using SciPy's Akima1DInterpolator. note
// that numpy.set_printoptions(floatmode="unique") should be used when printing
// the numpy.ndarray results in order to get maximum precision.
//
constexpr auto akima_spline_test_inputs = std::make_tuple(
  // interpolation only
  std::tuple{
    &AkimaSplineTest<>::spline_1,
    std::array{1.5, 2.5, 3.5, 4.5},
    std::tuple{
      std::array{3.725, 5.8, 5.0375, 4.125},
      std::array{2.95, 1., -2.825, 1.},
      std::array{
        -1.7999999999999998, -2.4000000000000004,
        -0.3000000000000007, 3.
      },
      std::array{1.200000000000001, 0., 19.799999999999997, 0.},
      std::array{0., 0., 0., 0.}
    }
  },
  // interpolation + extrapolation
  std::tuple{
    &AkimaSplineTest<>::spline_1,
    std::array{0.5, 2.2, 4.4, 5.5},
    std::tuple{
      std::array{-0.32500000000000007, 5.392, 4.04, 6.625},
      std::array{5.3500000000000005, 1.7199999999999998, 0.7000000000000011, 4.},
      std::array{-3.000000000000001, -2.4000000000000004, 3., 3.},
      std::array{1.200000000000001, 0., 0., 0.},
      std::array{0., 0., 0., 0.}
    }
  },
  // more complex interpolation
  std::tuple{
    &AkimaSplineTest<>::spline_2,
    std::array{-2.5, -1.5, -0.5, 0.5, 1.5, 2.5},
    std::tuple{
      std::array{
        4.225961538461538, 1.7091346153846156, -1.981118881118881,
        -1.026060606060606, 1.8297484276729556, 3.5548349056603774
      },
      std::array{
        3.3269230769230766, -7.052884615384613, -1.3468531468531468,
        2.238787878787879, 3.546163522012579, -1.434669811320755
      },
      std::array{
        -9.807692307692307, 0.3269230769230802, 3.848951048951049,
        1.0084848484848488, -0.23798742138364837, -5.638679245283019
      },
      std::array{
        -7.846153846153845, 37.269230769230774, 8.324475524475524,
        -0.9309090909090916, -8.307924528301884, 8.032075471698118
      },
      std::array{0., 0., 0., 0., 0., 0.}
    }
  },
  // piecewise polynomial interpolation using the knots points and data from
  // https://www.mathworks.com/help/matlab/ref/makima.html
  std::tuple{
    &AkimaSplineTest<>::spline_3,
    std::array{-4.5, -3.5, -2.5, -1.5, -0.5, 0.4, 1.4, 2.4, 3.4, 4.4},
    std::tuple{
      std::array{1., 1., 0.5625, -0.125, 0.5, 1.024, 1.424, 2.,  2., 2.},
      std::array{
        0., 0., -1.375, 0., 1.25,
        -0.22000000000000008, 1.38, 0., 0., 0.
      },
      std::array{
        0., 0., -0.5, 1., 0.,
        -0.5999999999999996, 0.4000000000000008, 0., 0., 0.
      },
      std::array{0., 0., 9., 0., -6., 6., -9., 0., 0., 0.},
      std::array{0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}
    }
  }
);
PDMATH_MSVC_WARNINGS_POP()

/**
 * Partial specialization for an Akima spline test.
 *
 * @tparam I Test input index
 */
template <std::size_t I>
class AkimaSplineTest<pdmath::testing::index<I>> : public ::testing::Test {
protected:
  // bounds check
  static_assert(I < std::tuple_size_v<decltype(akima_spline_test_inputs)>);
  // input tuple, spline factory, evaluation points, expected results tuple
  // note: although we could use auto& instead versions of MSVC before VS 2026
  // tend to materialize the value too late which leads to access violations
  static constexpr auto input_ = std::get<I>(akima_spline_test_inputs);
  static constexpr auto make_f_ = std::get<0>(input_);
  static constexpr auto xs_ = std::get<1>(input_);
  static constexpr auto res_ = std::get<2>(input_);

  /**
   * Return the range values as-is via forwarding.
   *
   * @tparam R Range type
   */
  template <typename R>
  static constexpr R&& values(
    R&& v,
    pdmath::constraint_t<pdmath::is_range_v<R>> = 0)
  {
    return std::forward<R>(v);
  }

  /**
   * Return the first element of the range-tag pair.
   *
   * @tparam R Range type
   * @tparam T Floating-point type
   */
  template <typename R, typename T>
  static constexpr auto& values(
    const std::pair<R, pdmath::testing::type_wrapper<T>>& v,
    pdmath::constraint_t<pdmath::is_range_v<R>> = 0)
  {
    return v.first;
  }

  /**
   * Return the default float matcher given a range.
   *
   * @tparam R Range type
   */
  template <typename R>
  static auto matcher(R&& /*v*/, pdmath::constraint_t<pdmath::is_range_v<R>> = 0)
  {
    return ::testing::DoubleEq();
  }

  /**
   * Return the matcher corresponding to the range-tag pair.
   *
   * @tparam R Range type
   * @tparam T Floating-point type
   */
  template <typename R, typename T>
  static auto matcher(
    const std::pair<R, pdmath::testing::type_wrapper<T>>& /*v*/,
    pdmath::constraint_t<
      pdmath::is_range_v<R> && std::is_floating_point_v<T> > = 0)
  {
    if constexpr (std::is_same_v<T, float>)
      return ::testing::FloatEq();
    else
      return ::testing::DoubleEq();
  }

  /**
   * Ctor.
   *
   * Constructs the Akima spline object used by the test case.
   */
  AkimaSplineTest() : f_{make_f_()} {}

  /**
   * Test that spline evaluations or derivatives match the expected values.
   *
   * `I = 0` gives spline evaluations and others are for higher derivatives.
   *
   * @tparam J Order of the derivative
   */
  template <std::size_t J>
  void test(pdmath::constraint_t<(J >= 0) && (J <= 4)> = 0) const
  {
    // expected values + matcher
    // note: could also be constexpr but see above for comment on MSVC bugs
    auto& v0s = values(std::get<J>(res_));
    auto m = matcher(std::get<J>(res_));
    // compute "projection" for std::transform
    // note: using decltype(auto) here for value-decay instead of reference
    decltype(auto) f = [this]() -> decltype(auto)
    {
      if constexpr (J == 0)
        return f_;
      // note: f_ is template type dependent so template keyword needed
      else
        return f_.template d<J>();
    }();
    // fill values and compare
    // note: using std::vector here for variety in ranges
    std::vector<decltype(f_(0))> vs(xs_.size());
    std::transform(xs_.begin(), xs_.end(), vs.begin(), f);
    EXPECT_THAT(vs, ::testing::Pointwise(m, v0s));
  }

  // spline object (member type deduced from input)
  decltype(make_f_()) f_;
};

// test input types
TYPED_TEST_SUITE(
  AkimaSplineTest,
  pdmath::testing::index_types_from_t<decltype(akima_spline_test_inputs)>
);

/**
 * Macro for defining a `TYPED_TEST(AkimaSplineTest, ...)` for the given order.
 *
 * If `i` is 0, spline evaluations are tested, otherwise derivatives.
 *
 * @param i Order of the derivative
 */
#define PDMATH_AKIMA_SPLINE_TEST(i) \
  /* note: cannot use macro expansion in TYPED_TEST() */ \
  TYPED_TEST(AkimaSplineTest, F ## i ## Test) \
  { \
      this->template test<i>(); \
  } \
  /* enforce semicolon usage */ \
  static_assert(true)

PDMATH_AKIMA_SPLINE_TEST(0);
PDMATH_AKIMA_SPLINE_TEST(1);
PDMATH_AKIMA_SPLINE_TEST(2);
PDMATH_AKIMA_SPLINE_TEST(3);
PDMATH_AKIMA_SPLINE_TEST(4);

#undef PDMATH_AKIMA_SPLINE_TEST

}  // namespace
