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

// convenience typedef for the himmelblau_functor
template <typename Tp_t>
using himmelblau_functor_t = pdmath::himmelblau_functor<
  typename Tp_t::gradient_type, typename Tp_t::hessian_type
>;

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
  static inline himmelblau_functor_t<Tp_t> himmel_;
};

// types LineSearchTest will be instantiated with + register types
using LineSearchTestTypes = ::testing::Types<
  pdmath::testing::func_type_pair<Eigen::Vector2f, Eigen::Matrix2f>,
  pdmath::testing::func_type_pair<Eigen::Vector2d, Eigen::Matrix2d>,
  pdmath::testing::func_type_pair<pdmath::array_pair_f, Eigen::MatrixXf>,
  pdmath::testing::func_type_pair<pdmath::vector_d, Eigen::Matrix2d>
>;
TYPED_TEST_SUITE(LineSearchTest, LineSearchTestTypes);

TYPED_TEST(LineSearchTest, DISABLED_HimmelblauTest) {}

}  // namespace
