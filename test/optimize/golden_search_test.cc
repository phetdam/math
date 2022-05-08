/**
 * @file golden_search_test.cc
 * @author Derek huang <djh458@stern.nyu.edu>
 * @brief Test for golden_search, validating that it works correctly
 */

#include <cmath>
#include <limits>
#include <utility>

#include <boost/math/tools/roots.hpp>
#include <gtest/gtest.h>

#include "pdmath/optimize/functor_base.h"
#include "pdmath/optimize/golden_search.h"
#include "pdmath/optimize/optimize_result.h"

namespace pdmath {
namespace tests {
namespace optimize {

namespace {

template <class T = double>
class scalar_quadratic : public pdmath::optimize::functor_base<T> {
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
  T operator()(const T& x) { return a_ * std::pow(x, 2) + b_ * x + c_; }

private:
  T a_;
  T b_;
  T c_;
};

}  // namespace

// note: GoogleTest test suite/test name CANNOT contain underscores! hence the
// sudden change to using CamelCase instead of iso_standard
TEST(GoldenSearchTests, TestCorrectness)
{
  // as we all know, the root of this is 2
  scalar_quadratic quad_func(1., -4., 4.);
  // same as default tolerance; in general a good choice on any system
  double tol = std::sqrt(std::numeric_limits<double>::epsilon());

  pdmath::optimize::scalar_optimize_result res(
    std::move(pdmath::optimize::golden_search(quad_func, 1., 3., tol))
  );

  // always converges
  EXPECT_TRUE(res.converged());
  EXPECT_NEAR(2., res.res(), tol);
  // n_fev should be 4 + 2 * n_iter
  EXPECT_EQ(4 + 2 * res.n_iter(), res.n_fev());
}

}  // namespace pdmath::tests::optimize
}  // namespace pdmath::tests
}  // namespace pdmath
