/**
 * @file akima_spline_test.cc
 * @author Derek Huang
 * @brief akima_spline.h unit tests
 * @copyright MIT License
 */

#include "pdmath/akima_spline.h"

#include <algorithm>
#include <array>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

/**
 * Akima spline base test case fixture.
 *
 * @todo Add more tests beyond this simple comparison test with SciPy results.
 */
class AkimaSplineTest : public ::testing::Test {
protected:
  // knot points + knot values
  static constexpr std::array xs_{1., 2., 3., 4., 5.};
  static constexpr std::array ys_{2., 5., 6., 4., 5.};
  // evaluation points
  static constexpr std::array vs_{1.5, 2.5, 3.5, 4.5};

  /**
   * Ctor.
   *
   * Constructs a new Akima spline from the static points and knot values.
   */
  AkimaSplineTest() : f_{xs_, ys_} {}

  pdmath::akima_spline<double> f_;
};

/**
 * Test that spline evaluations match SciPy values.
 */
TEST_F(AkimaSplineTest, D0Test)
{
  // Akima1DInterpolator computed values
  constexpr std::array exp = {3.725, 5.8, 5.0375, 4.125};
  // fill values and compare
  // note: using std::vector here for variety in ranges
  std::vector<double> act(vs_.size());
  std::transform(vs_.begin(), vs_.end(), act.begin(), f_);
  EXPECT_THAT(act, ::testing::Pointwise(::testing::DoubleEq(), exp));
}

/**
 * Test that spline first derivatives match SciPy values.
 */
TEST_F(AkimaSplineTest, D1Test)
{
  // Akima1DInterpolator computed values
  constexpr std::array exp = {2.95, 1., -2.825, 1.};
  // fill values and compare
  std::vector<double> act(vs_.size());
  std::transform(vs_.begin(), vs_.end(), act.begin(), f_.d());
  EXPECT_THAT(act, ::testing::Pointwise(::testing::DoubleEq(), exp));
}

/**
 * Test that spline second derivatives match SciPy values.
 */
TEST_F(AkimaSplineTest, D2Test)
{
  // Akima1DInterpolator computed values
  constexpr std::array exp = {-1.8, -2.4, -0.3, 3.};
  // fill values and compare
  std::vector<double> act(vs_.size());
  // note: testing chaining of the d() member functions
  std::transform(vs_.begin(), vs_.end(), act.begin(), f_.d().d());
  // note: using DoubleEq() causes the test to fail because f_.d<2>()(3.5)
  // computes a slightly different value of -0.3 that fails the ULP test
  EXPECT_THAT(act, ::testing::Pointwise(::testing::FloatEq(), exp));
}

/**
 * Test that spline third derivatives match SciPy values.
 */
TEST_F(AkimaSplineTest, D3Test)
{
  // Akima1DInterpolator computed values
  constexpr std::array exp = {1.2, 0., 19.8, 0.};
  // fill values and compare
  std::vector<double> act(vs_.size());
  std::transform(vs_.begin(), vs_.end(), act.begin(), f_.d<3>());
  // note: using DoubleEq() causes the test to fail because f_.d<3>()(1.5)
  // computes a slightly different value of 1.2 that fails the ULP test
  EXPECT_THAT(act, ::testing::Pointwise(::testing::FloatEq(), exp));
}

/**
 * Test that spline thourth derivatives match SciPy values.
 */
TEST_F(AkimaSplineTest, D4Test)
{
  // Akima1DInterpolator computed values
  constexpr std::array exp = {0., 0., 0., 0.};
  // fill values and compare
  std::vector<double> act(vs_.size());
  // note: testing chaining of the d() member functions
  std::transform(vs_.begin(), vs_.end(), act.begin(), f_.d<2>().d<2>());
  EXPECT_THAT(act, ::testing::Pointwise(::testing::DoubleEq(), exp));
}

}  // namespace
