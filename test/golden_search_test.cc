/**
 * @file golden_search_test.cc
 * @author Derek huang <djh458@stern.nyu.edu>
 * @brief Test for golden_search, validating that it works correctly
 * @copyright MIT License
 */

#include "pdmath/golden_search.h"

#include <limits>
#include <utility>

#include <gtest/gtest.h>

#include "pdmath/bases.h"
#include "pdmath/optimize_result.h"

namespace pdmath {
namespace tests {

namespace {

template <class T = double>
class scalar_quadratic : public functor_base<T> {
public:
  /**
   * Constructor for the functor.
   *
   * @param a Value scaling the quadratic term
   * @param b Value scaling the linear term
   * @param c Offset
   */
  scalar_quadratic(const T& a, const T& b, const T& c) : a_(a), b_(b), c_(c) {}

  /**
   * Evaluate for a given value.
   */
  T operator()(const T& x) { return a_ * x * x + b_ * x + c_; }

private:
  T a_;
  T b_;
  T c_;
};

// note: GoogleTest test suite/test name CANNOT contain underscores! hence the
// sudden change to using CamelCase instead of iso_standard
TEST(GoldenSearchTest, CorrectnessTest)
{
  // as we all know, the root of this is 2
  scalar_quadratic quad_func(1., -4., 4.);
  // same as default tolerance; in general a good choice on any system
  double tol = std::sqrt(std::numeric_limits<double>::epsilon());
  auto res = std::move(pdmath::golden_search(quad_func, 1., 3., tol));
  // always converges
  EXPECT_TRUE(res.converged());
  EXPECT_NEAR(2., res.res(), tol);
  // n_fev should be 4 + 2 * n_iter
  EXPECT_EQ(4 + 2 * res.n_iter(), res.n_fev());
}

}  // namespace

}  // namespace pdmath::tests
}  // namespace pdmath
