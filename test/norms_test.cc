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

#include "pdmath/helpers.h"
#include "pdmath/types.h"

namespace pdmath {
namespace tests {

namespace {


/**
 * Max norm implementation using the `norm` template class.
 *
 * @tparam T scalar type
 * @tparam C_t compound type with `T` elements, ex. vector or matrix type
 */
template <class T = double, class C_t = std::vector<T>>
class max_norm : public norm<T, C_t> {
public:
  /**
   * Return max norm of `x`.
   */
  T operator()(const C_t& x)
  {
    return std::abs(*std::max_element(x.begin(), x.end(), max_comp));
  }

private:
  /**
   * Comparison function used by `std::max_element` in `operator()`.
   *
   * @param a `const T&` first value
   * @param b `const T&` second value
   * @returns `true` if `std::abs(a) < std::abs(b)` else `false`
   */
  static bool max_comp(const T& a, const T& b)
  {
    return std::abs(a) < std::abs(b);
  }
};

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
  max_norm norm;
  ASSERT_DOUBLE_EQ(8.1, norm(values_));
}

/**
 * Test that the `p_norm` functor works as intended for different `p` values.
 */
TEST_P(NormTest, PNormTest)
{
  auto p = GetParam();
  p_norm norm(p);
  // compute expected norm
  double exp_norm = 0;
  for (const auto& value : values_) {
    exp_norm += std::abs(std::pow(value, p));
  }
  exp_norm = (p) ? std::pow(exp_norm, 1. / p) : exp_norm;
  // compare against actual
  ASSERT_DOUBLE_EQ(exp_norm, norm(boost_vector_from(values_)));
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
