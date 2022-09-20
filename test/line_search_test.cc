/**
 * @file line_search_test.cc
 * @author Derek Huang
 * @brief Tests for the line search method and supporting classes
 * @copyright MIT License
 */

#include "pdmath/line_search.h"

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "pdmath/math_functors.h"
#include "pdmath/testing/macros.h"
#include "pdmath/testing/utils.h"

namespace {

// using declaration for functors dependent on a func_type_triple
template <typename Tr_t>
using tr_himmelblau_functor = pdmath::himmelblau_functor<
  typename Tr_t::scalar_t, typename Tr_t::vector_t, typename Tr_t::matrix_t
>;

/**
 * Test fixture for our line search tests.
 *
 * @tparam Tr_t A specialization of `func_type_triple`
 */
template <typename Tr_t>
class LineSearchTest : public ::testing::Test {
protected:
  LineSearchTest() : himmel_() {}
  tr_himmelblau_functor<Tr_t> himmel_;
  // pdmath::steepest_direction_search dir_search_;
};

// types LineSearchTest will be instantiated with + register types
using LineSearchTestTypes = ::testing::Types<
  pdmath::testing::func_type_triple<float, Eigen::Vector2f, Eigen::Matrix2f>,
  pdmath::testing::func_type_triple<double, Eigen::Vector2d, Eigen::Matrix2d>,
  pdmath::testing::func_type_triple<double, pdmath::vector_d, Eigen::Matrix2d>
>;
TYPED_TEST_SUITE(LineSearchTest, LineSearchTestTypes);

// TYPED_TEST(LineSearchTest, HimmelblauTest)
// {

// }

}  // namespace
