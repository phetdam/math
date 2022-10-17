/**
 * @file line_search_test.cc
 * @author Derek Huang
 * @brief Tests for the line search method and supporting classes
 * @copyright MIT License
 */

#include "pdmath/line_search.h"

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "pdmath/helpers.h"
#include "pdmath/math_functors.h"
#include "pdmath/norms.h"
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
  /**
   * Constructor to populate non-shared members that will have state mutation.
   */
// MSVC complains that double (0.1) in backtrack_step_ is truncated to float
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4305)
#endif  // _MSC_VER
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
#pragma warning (pop)
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
  // should increase gradient eval count to 1
  EXPECT_EQ(1, this->steepest_search_.n_gev());
}

TYPED_TEST(LineSearchTest, DISABLED_HimmelblauTest)
{
  (void) 0;
}

}  // namespace
