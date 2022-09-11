/**
 * @file norms_tests.cc
 * @author Derek Huang
 * @brief Tests for norm functors
 * @copyright MIT License
 */

#include "pdmath/norms.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "pdmath/helpers.h"
#include "pdmath/types.h"

namespace {

/**
 * Magnitude of largest value in `NORMS_TEST_VECTOR_VALUES`.
 */
#define NORMS_TEST_VALUES_ABS_MAX 8.1

/**
 * Values used in the `NormsTest` test fixture for populating `std::vector`.
 *
 * Used later in `MaxNormTest` test to test the `boost_vector<double>` and
 * `eigen_vector<double>` specializations of `max_norm`.
 */
#define NORMS_TEST_VECTOR_VALUES -5.7, 6, -NORMS_TEST_VALUES_ABS_MAX, 1.3

/**
 * Templated test fixture for norm tests.
 *
 * Parametrized for p-norm tests.
 */
class NormsTest : public ::testing::TestWithParam<unsigned int> {
protected:
  static const std::vector<double> values_;
  static constexpr double values_abs_max_ = NORMS_TEST_VALUES_ABS_MAX;
};

const std::vector<double> NormsTest::values_ = {NORMS_TEST_VECTOR_VALUES};

/**
 * `MaxNormTest` macro checking if `x` equals` `NORMS_TEST_VALUES_ABS_MAX`.
 *
 * @param x `double` value to compare against `NORMS_TEST_VALUES_ABS_MAX`.
 */
#define EXPECT_EQ_NORMS_TEST_VALUES_ABS_MAX(x) \
  EXPECT_DOUBLE_EQ(NORMS_TEST_VALUES_ABS_MAX, x)

/**
 * Test that a subclassed `norm` works as intended, here using max norm.
 *
 * We try STL, Boost, Eigen vector specializations.
 */
TEST_F(NormsTest, MaxNormTest)
{
  pdmath::max_norm<double, pdmath::vector_d> std_norm;
  pdmath::max_norm<double, pdmath::boost_vector_d> boost_norm;
  pdmath::max_norm<double, Eigen::VectorXd> eigen_norm;
  auto bvalues = pdmath::boost_vector_from<double>(NORMS_TEST_VECTOR_VALUES);
  auto evalues = pdmath::eigen_vector_from<double>(NORMS_TEST_VECTOR_VALUES);
  EXPECT_EQ_NORMS_TEST_VALUES_ABS_MAX(std_norm(values_));
  EXPECT_EQ_NORMS_TEST_VALUES_ABS_MAX(boost_norm(bvalues));
  EXPECT_EQ_NORMS_TEST_VALUES_ABS_MAX(eigen_norm(evalues));
}

/**
 * Test that the `p_norm` functor works as intended for different `p` values.
 */
TEST_P(NormsTest, PNormTest)
{
  auto p = GetParam();
  pdmath::p_norm<double, pdmath::vector_d> norm(p);
  // compute expected norm
  double exp_norm = 0;
  for (const auto& value : values_) {
    exp_norm += std::abs(std::pow(value, p));
  }
  exp_norm = (p) ? std::pow(exp_norm, 1. / p) : exp_norm;
  // compare against actual
  ASSERT_DOUBLE_EQ(exp_norm, norm(values_));
}

INSTANTIATE_TEST_SUITE_P(
  PNorm,
  NormsTest,
  // Range(a, b) will only generate a, a + 1, ... b - 1 inclusive
  ::testing::Range(0u, 4u),
  ::testing::PrintToStringParamName()
);

}  // namespace
