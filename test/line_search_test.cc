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
#include "pdmath/testing/macros.h"
#include "pdmath/testing/utils.h"

namespace {

/**
 * Test fixture for our line search tests.
 *
 * @tparam Tp_t A specialization of `func_type_pair`
 */
template <typename Tp_t>
class LineSearchTest : public ::testing::Test {
public:
  using gradient_type = typename Tp_t::gradient_type;
  using hessian_type = typename Tp_t::hessian_type;

protected:
  // functor to minimize (chosen for convenience)
  static inline pdmath::himmelblau_functor<gradient_type, hessian_type> himmel_;
  // direction search using steepest (gradient) descent
  static inline pdmath::steepest_direction_search<gradient_type> steepest_dir_{
    himmel_.d1
  };
  // step search policies: constant step size, backtrack search
  static inline pdmath::const_step_search<gradient_type> const_step_;
  static inline pdmath::backtrack_step_search<gradient_type> backtrack_step_{
    himmel_.f, himmel_.d1, 0.1
  };
  // convergence policy based on line search direction ()
  static inline pdmath::no_direction_policy<gradient_type> policy_;
};

// types LineSearchTest will be instantiated with + register types
using LineSearchTestTypes = ::testing::Types<
  pdmath::testing::func_type_pair<Eigen::Vector2f, Eigen::Matrix2f>,
  pdmath::testing::func_type_pair<Eigen::Vector2d, Eigen::Matrix2d>,
  pdmath::testing::func_type_pair<pdmath::array_pair_f, Eigen::MatrixXf>,
  pdmath::testing::func_type_pair<pdmath::vector_d, Eigen::Matrix2d>
>;
TYPED_TEST_SUITE(LineSearchTest, LineSearchTestTypes);

TYPED_TEST(LineSearchTest, DISABLED_HimmelblauTest)
{
  (void) 0;
}

}  // namespace
