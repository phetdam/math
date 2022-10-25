/**
 * @file line_search_test.cc
 * @author Derek Huang
 * @brief Tests for the line search method and supporting classes
 * @copyright MIT License
 */

#include "pdmath/line_search.h"

#include <Eigen/Core>
#include <Eigen/LU>
#include <gtest/gtest.h>

#include "pdmath/helpers.h"
#include "pdmath/math_functors.h"
#include "pdmath/norms.h"
#include "pdmath/warnings.h"
#include "pdmath/testing/macros.h"
#include "pdmath/testing/utils.h"

namespace {

/**
 * Test fixture for our line search tests.
 *
 * @tparam Tp_t `func_type_pair` specialization
 */
template <typename Tp_t>
class LineSearchTest
  : public ::testing::Test,
    public pdmath::testing::tol_mixin<typename Tp_t::scalar_type> {
private:
  using V_t = typename Tp_t::gradient_type;
  using H_t = typename Tp_t::hessian_type;

public:
  PDMATH_USING_FUNCTOR_TYPES(V_t, H_t);

protected:
// MSVC complains that double (0.1) in backtrack_step_ is truncated to float
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
  /**
   * Constructor to populate non-shared members that will have state mutation.
   */
  LineSearchTest()
    : steepest_search_{
        // note no capture needed, as himmel_ is static. lambda is used to get
        //around member functions being different types than normal functions.
        [](const gradient_type& x) { return himmel_.d1(x); }
      },
      backtrack_step_{
        [](const gradient_type& x) { return himmel_.f(x); },
        [](const gradient_type& x) { return himmel_.d1(x); },
        0.1
      }
  {}
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER

  // direction search using steepest (gradient) descent
  pdmath::steepest_direction_search<gradient_type> steepest_search_;
  // step search policy using backtracking step search
  pdmath::backtrack_step_search<gradient_type> backtrack_step_;

  // functor to minimize (chosen for convenience)
  static inline pdmath::himmelblau_functor<gradient_type, hessian_type> himmel_;
  // step search policy using constant step size
  static inline pdmath::const_step_search<gradient_type> const_step_;
  // line search direction convergence policies: no early convergence, 2-norm
  static inline pdmath::no_direction_policy<gradient_type> no_policy_;
  static inline pdmath::min_p_norm_direction_policy<gradient_type> min_policy_;
  // starting point of (0, 0)
  static inline const gradient_type x0_{{0, 0}};
  // first Himmelblau minimum
  static inline const gradient_type min_1_{{HIMMELBLAU_ZERO_1}};
};

// types LineSearchTest will be instantiated with + register types
using LineSearchTestTypes = ::testing::Types<
  pdmath::testing::func_type_pair<Eigen::Vector2f, Eigen::Matrix2f>,
  pdmath::testing::func_type_pair<Eigen::Vector2d, Eigen::Matrix2d>,
  pdmath::testing::func_type_pair<pdmath::array_pair_f, Eigen::MatrixXf>,
  pdmath::testing::func_type_pair<pdmath::vector_d, Eigen::Matrix2d>
>;
TYPED_TEST_SUITE(LineSearchTest, LineSearchTestTypes);

/**
 * Test that the `steepest_direction_search` functor works as expected.
 *
 * Uses `HIMMELBLAU_ZERO_1` to check that the gradient is zero (should actually
 * be exactly zero) and checks function, gradient, Hessian evals.
 */
TYPED_TEST(LineSearchTest, SteepestDirectionSearchTest)
{
  EXPECT_THAT(
    this->steepest_search_(TestFixture::min_1_),
    TestFixture::all_near_zero_matcher
  );
  // should increase gradient eval count to 1, no function or Hessian evals
  EXPECT_EQ(0, this->steepest_search_.n_fev());
  EXPECT_EQ(1, this->steepest_search_.n_gev());
  EXPECT_EQ(0, this->steepest_search_.n_hev());
}

/**
 * Test that `min_norm_direction_policy` works as expected.
 */
TYPED_TEST(LineSearchTest, MinNormDirectionPolicyTest)
{
  using scalar_type = typename TestFixture::scalar_type;
  using gradient_type = typename TestFixture::gradient_type;
  const auto& min_norm = TestFixture::min_policy_.min_norm();
  // norm will be a bit above min_policy_.min_norm() if scale is 0.8
// MSVC complains about double truncation to float
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
  scalar_type scale_a{0.8};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
  gradient_type x_a{{-scale_a * min_norm, scale_a * min_norm}};
  EXPECT_FALSE(TestFixture::min_policy_(x_a));
  // norm will be a bit below min_policy_.min_norm() if scale is 0.7
// MSVC complains about double truncation to float
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
  scalar_type scale_b{0.7};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
  gradient_type x_b{{-scale_b * min_norm, scale_b * min_norm}};
  EXPECT_TRUE(TestFixture::min_policy_(x_b));
}

/**
 * Test that the `const_step_search` works as expected.
 */
TYPED_TEST(LineSearchTest, ConstStepSearchTest)
{
  // before we call operator(), last_step() should be 0
  EXPECT_EQ(0, TestFixture::const_step_.last_step());
  // now should be exactly equal since the last step has been ovewritten. note
  // that const_step_search::operator() ignores inputs
  EXPECT_EQ(
    TestFixture::const_step_.last_step(),
    TestFixture::const_step_(TestFixture::x0_, TestFixture::min_1_)
  );
  // no function, gradient, Hessian evals
  EXPECT_EQ(0, TestFixture::const_step_.n_fev());
  EXPECT_EQ(0, TestFixture::const_step_.n_gev());
  EXPECT_EQ(0, TestFixture::const_step_.n_hev());
}

/**
 * Test that `backtrack_step_search` works as expected.
 *
 * Since the Himmelblau function is relatively simple, we take advantage of the
 * fact that if we start from the origin, `min_1_` is a descent direction.
 */
TYPED_TEST(LineSearchTest, BacktrackStepSearchTest)
{
  using scalar_type = typename TestFixture::scalar_type;
  using gradient_type = typename TestFixture::gradient_type;
  // step is taken exactly in the direction of the first minimum
  const auto t_x{this->backtrack_step_(TestFixture::x0_, TestFixture::min_1_)};
  // since min_1_ is the minimum itself and Himmelblau's function is decreasing
  // to min_1_, step should be the same as what's returned by eta0()
  EXPECT_NEAR(this->backtrack_step_.eta0(), t_x, this->tol());
  // we define the expected step t_se and start at min_1_ - t_se * 1
  scalar_type t_se{
    this->backtrack_step_.eta0() *
    this->backtrack_step_.rho() * this->backtrack_step_.rho()
  };
  // modify element-by-element, not all Containers define operator-, operator+
  gradient_type x_s = TestFixture::min_1_;
  x_s[0] -= t_se * x_s[0];
  x_s[1] -= t_se * x_s[1];
  // actual step should equal t_se as we are t_se away from min_1_
  const auto t_s{this->backtrack_step_(x_s, TestFixture::min_1_)};
  EXPECT_NEAR(t_se, t_s, this->tol());
  // should only be 2 gradient evaluations, 1 for each invocation, and there
  // should be 2 + 1 + 3 function evaluations in total
  EXPECT_EQ(6, this->backtrack_step_.n_fev());
  EXPECT_EQ(2, this->backtrack_step_.n_gev());
  EXPECT_EQ(0, this->backtrack_step_.n_hev());
}

/**
 * Check that steepest descent on the Himmelblau functor works as expected.
 *
 * When starting from the origin, the expected minimum reached is min_1_, i.e.
 * HIMMELBLAU_ZERO_1, which should be relatively easy to reach. We check that
 * the result is as expected, that the gradient is close to 0, and that the
 * Hessian is positive semidefinite (determinant is nonnegative).
 */
TYPED_TEST(LineSearchTest, HimmelblauSteepestDescentTest)
{
  // expected minimum when starting from origin is min_1_
  const auto res = pdmath::line_search(
    TestFixture::himmel_,
    this->steepest_search_,
    this->backtrack_step_,
    TestFixture::x0_,
    100,
    TestFixture::min_policy_
  );
  // check that result is close with min_1_ by subtracting them and checking
  // that the max norm of the difference and gradient is close enough to zero
  auto x_diff = res.res();
  std::transform(
    x_diff.cbegin(),
    x_diff.cend(),
    TestFixture::min_1_.cbegin(),
    x_diff.begin(),
    [&](const auto& x, const auto& y) { return x - y; }
  );
  EXPECT_THAT(x_diff, TestFixture::all_near_zero_matcher);
  EXPECT_THAT(res.grad(), TestFixture::all_near_zero_matcher);
  // determinant of the Hessian should be nonnegative
  EXPECT_GE(res.hess().determinant(), 0);
}

}  // namespace
