/**
 * @file natural_spline_test.c
 * @author Derek Huang
 * @brief natural_spline.h unit tests
 * @copyright MIT License
 */

#include "pdmath/natural_spline.h"

#include <array>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/functional.h"
#include "pdmath/testing/utils.h"
#include "pdmath/type_traits.h"
#include "pdmath/warnings.h"

namespace {

/**
 * `natural_spline` template test fixture.
 *
 * @tparam T Input type
 */
template <typename T = void>
class NaturalSplineTest : public ::testing::Test {};

/**
 * Base specialization for the `NaturalSplineTest`.
 *
 * This holds some shared spline creation functions.
 */
template <>
class NaturalSplineTest<void> : public ::testing::Test {
public:
  /**
   * Return a natural cubic spline over 5 knot points.
   *
   * This uses the same knots and values as `AkimaSplineTest<>::spline_1()`.
   */
  static auto spline_1()
  {
    return pdmath::natural_spline{
      {1., 2., 3., 4., 5.},
      {2., 5., 6., 4., 5.}
    };
  }

  /**
   * Return a natural cubic spline over 7 knot points.
   *
   * This uses the same knots and values as `AkimaSplineTest<>::spline_2()`.
   */
  static auto spline_2()
  {
    // note: initializer lists need not both have floating-point value types
    return pdmath::natural_spline{
      {-3, -2, -1, 0, 1, 2, 3},
      {1.5, 4.5, -1., -2., 0.2, 3.4, 2.3}
    };
  }

  /**
   * Return a natural cubic spline for the MATLAB example piecewise polynomial.
   *
   * Knot data is from https://www.mathworks.com/help/matlab/ref/makima.html.
   */
  static auto spline_3()
  {
    // note: initializer lists need not both have floating-point value types
    return pdmath::natural_spline{
      {-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5},
      {1., 1., 1., 0., 0., 1., 1., 2., 2., 2., 2.}
    };
  }
};

// natural spline test case inputs. each input is a std::tuple with the format:
//
// 1. nullary invocable returning a natural_spline
// 2. std::array evaluation points (note: because std::array is constexpr)
// 3. result specification std::tuple<T0, T1, T2, T3, T4>
// 4. [optional] absolute comparison tolerance
//
// for 3. each Tk corresponds to the kth derivative of the spline, where 0 is
// simply the spline's evaluation points. Tk is a std::array of values. for 4.
// the tolerance value will not be used to deduce the matcher type.
//
// all expected values were computed using SciPy's CubicSpline with
// bc_type="natural" and extrapolate=True (when extrapolating).
//
constexpr auto natural_spline_test_inputs = std::make_tuple(
  // interpolation only
  std::tuple{
    &NaturalSplineTest<>::spline_1,
// suppress MSVC C5246 due to old interpretation of std::array initialization
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5246)
    // note: same points as the Akima spline test case for comparison
    std::array{1.5, 2.5, 3.5, 3.5},
    std::tuple{
      std::array{
        3.6004464285714284, 5.948660714285714,
        4.979910714285714, 4.979910714285714
      },
      std::array{
        3.0669642857142856, 1.1651785714285714,
        -2.477678571428572, -2.477678571428572
      },
      std::array{
        -0.8035714285714279, -3.5892857142857144,
        0.1607142857142847, 0.1607142857142847
      },
      std::array{
        -1.607142857142854, -3.964285714285712,
        11.464285714285715, 11.464285714285715
      },
      std::array{0., 0., 0., 0.}
    },
    // note: override needed as f.d<2>() is off by at most 6.66134e-16 (at 1.5)
    // and f.d<3>() is off by at most 3.10862e-15 (at 1.5). internally, SciPy's
    // CubicSpline uses LAPACK's gtsv to solve the resulting tridiagonal system
    // using partial pivoting Gaussian elimination, not Thomas' algorithm, so
    // some numerical differences are not wholly unexpected
    4e-15
  },
  // interpolation + extrapolation
  std::tuple{
    &NaturalSplineTest<>::spline_1,
    // note: same points as the Akima spline test case for comparison
    std::array{0.5, 2.2, 4.4, 5.5},
    std::tuple{
      std::array{
        0.3995535714285713, 5.455428571428572,
        4.022857142857143, 5.868303571428572
      },
      std::array{
        3.0669642857142865, 2.063571428571428,
        0.9214285714285727, 1.2455357142857144
      },
      std::array{
        0.8035714285714262, -2.4000000000000012,
        3.535714285714284, -2.946428571428572
      },
      std::array{
        -1.607142857142854, -3.964285714285712,
        -5.892857142857143, -5.892857142857143
      },
      std::array{0., 0., 0., 0.}
    },
    // note: override needed as f is off by at most 8.88178e-16 (at 5.5),
    // f.d<1>() off by at most 8.88178e-16 (at 0.5), f.d<2>() off by at most
    // -2.44249e-15 (at 0.5), f.d<3>() off by at most 3.10862e-15 (at 0.5). see
    // above comment about SciPy's CubicSpline using LAPACK's gtsv
    4e-15
  },
  // more complex interpolation
  std::tuple{
    &NaturalSplineTest<>::spline_2,
    // note: same points as the Akima spline test case for comparison
    std::array{-2.5, -1.5, -0.5, 0.5, 1.5, 2.5},
    std::tuple{
      std::array{
        3.9561538461538457, 2.0690384615384616, -2.2323076923076925,
        -1.1773076923076922, 2.066538461538461, 3.2986538461538464
      },
      std::array{
        3.637435897435898, -6.5621794871794865, -0.6387179487179486,
        2.1420512820512823, 3.6205128205128196, -1.3991025641025643
      },
      std::array{
        -7.649230769230768, -2.5523076923076946, 5.8584615384615395,
        2.218461538461538, -2.132307692307693, -3.5892307692307686
      },
      std::array{
        -15.298461538461538, 25.492307692307687, -8.670769230769233,
        1.39076923076923, -10.092307692307688, 7.178461538461537
      },
      std::array{0., 0., 0., 0., 0., 0.}
    }
  },
  // piecewise polynomial interpolation using the knots points and data from
  // https://www.mathworks.com/help/matlab/ref/makima.html
  std::tuple{
    &NaturalSplineTest<>::spline_3,
    std::array{-4.5, -3.5, -2.5, -1.5, -0.5, 0.4, 1.4, 2.4, 3.4, 4.4},
    std::tuple{
      std::array{
        0.9684889568849296, 1.0945331293452114,
        0.5283785257342251, -0.20804723228211164,
        0.5538104033942214, 1.014570408945518,
        1.3556330857278807, 2.1148972481429587,
        1.968777921700283, 2.0099910650559094
      },
      std::array{
        -0.021007362076713637, 0.10503681038356816,
        -1.1491398794575591, -0.0084772925533321,
        1.1830490496708874, -0.20262814243041072,
         1.166381083295884, -0.022896190753125906,
         0.005203679716619489, 0.002081471886647779
      },
      std::array{
        0.25208834492056353, -0.7562650347616906,
        -0.2270282058738009, 1.664377858256893,
        -0.4304832271537711, -0.4793703243543317,
        0.7086468053609671, -1.155216897089535,
        0.31222078299717154, -0.0936662348991514
      },
      std::array{
        0.5041766898411271, -2.5208834492056353,
        3.5793571069814165, 0.2034550212799704,
        -4.393177192101298, 5.369253747125221,
        -5.083837796399589, 2.9660974384731293,
        -0.7805519574929287, 0.15611039149858574
      },
      std::array{0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}
PDMATH_MSVC_WARNINGS_POP()
    },
    // note: override needed as f.d<2>() off by at most -3.33067e-16 (at 2.5)
    // and f.d<3>() off by at most -8.88178e-16 (at 1.4). see above comment
    9e-16
  }
);

/**
 * Partial specialization for a natural cubic spline test.
 *
 * @tparam I Input index
 */
template <std::size_t I>
class NaturalSplineTest<pdmath::testing::index<I>> : public ::testing::Test {
protected:
  static_assert(I < std::tuple_size_v<decltype(natural_spline_test_inputs)>);
  // input tuple, spline factory, evaluation points, expected results tuple
  // note: pre-VS 2026 MSVC tends to materialize values too late with constexpr
  // references so using auto& here leads to access violations
  static constexpr auto input_ = std::get<I>(natural_spline_test_inputs);
  static constexpr auto make_f_ = std::get<0>(input_);
  static constexpr auto xs_ = std::get<1>(input_);
  static constexpr auto res_ = std::get<2>(input_);

  /**
   * Return the standard GoogleMock matcher for the given type.
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

  /**
   * Ctor.
   *
   * Constructs the natural cubic spline object used by the test case.
   */
  NaturalSplineTest() : f_{make_f_()} {}

  /**
   * Test that spline evaluations or derivatives match their expected values.
   *
   * `J = 0` gives the spline evaluations and others are for derivatives.
   *
   * @tparam J Order of the derivative
   */
  template <std::size_t J>
  void test() const
  {
    static_assert(J < std::tuple_size_v<decltype(res_)>);
    // expected values
    auto& v0s = std::get<J>(res_);
    // get projection invocable for std::transform
    // note: use decltype(auto) to preserve value-decay
    decltype(auto) f = [this]() -> decltype(auto)
    {
      if constexpr (J == 0)
        return f_;
      // note: f_ is template type dependent so template keyword is needed
      else
        return f_.template d<J>();
    }();
    // get appropriate matcher
    auto m = []
    {
      using T = pdmath::range_value_t<decltype(v0s)>;
      // override provided in input_
      if constexpr (3 < std::tuple_size_v<decltype(input_)>)
        return matcher<T>(std::get<3>(input_));
      else
        return matcher<T>();
    }();
    // fill values and compare
    std::vector vs(xs_.begin(), xs_.end());
    pdmath::for_each(vs, [&f](auto& v) { v = f(v); });
    EXPECT_THAT(vs, ::testing::Pointwise(m, v0s));
  }

  // spline object (member type deduced from input)
  decltype(make_f_()) f_;
};

// test input types
TYPED_TEST_SUITE(
  NaturalSplineTest,
  pdmath::testing::index_types_from_t<decltype(natural_spline_test_inputs)>
);

/**
 * Macro defining a `TYPED_TEST(NaturalSplineTest, ...)` for the given order.
 *
 * If `i` is 0, spline evaluations are tested, otherwise derivatives.
 *
 * @param i Order of the derivative
 */
#define PDMATH_NATURAL_SPLINE_TEST(i) \
  /* note: cannot use macro expansion in TYPED_TEST() */ \
  TYPED_TEST(NaturalSplineTest, F ## i ## Test) \
  { \
      this->template test<i>(); \
  } \
  /* enforce semicolon usage */ \
  static_assert(true)

PDMATH_NATURAL_SPLINE_TEST(0);
PDMATH_NATURAL_SPLINE_TEST(1);
PDMATH_NATURAL_SPLINE_TEST(2);
PDMATH_NATURAL_SPLINE_TEST(3);
PDMATH_NATURAL_SPLINE_TEST(4);

#undef PDMATH_NATURAL_SPLINE_TEST

}  // namespace
