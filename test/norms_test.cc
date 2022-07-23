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

#include <gtest/gtest.h>

#include "pdmath/types.h"

namespace pdmath {
namespace tests {

namespace {

/**
 * Test fixture for norm tests.
 *
 * Parametrized for p-norm tests.
 */
class NormTest : public ::testing::TestWithParam<unsigned int> {
protected:
  static const std::vector<double> values_;
};

const std::vector<double> NormTest::values_ = {-5.7, 6, -8.1, 1.3};

/**
 * Test that a subclassed `norm` works as intended, here using max norm.
 */
TEST_F(NormTest, MaxNormTest)
{
  max_norm<double, pdmath::double_vector> norm;
  ASSERT_DOUBLE_EQ(8.1, norm(values_));
}

/**
 * Test that the `p_norm` functor works as intended for different `p` values.
 */
TEST_P(NormTest, PNormTest)
{
  auto p = GetParam();
  p_norm<double, pdmath::double_vector> norm(p);
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
  NormTest,
  // Range(a, b) will only generate a, a + 1, ... b - 1 inclusive
  ::testing::Range(0u, 4u),
  ::testing::PrintToStringParamName()
);

}  // namespace

}  // namespace pdmath::tests
}  // namespace pdmath
