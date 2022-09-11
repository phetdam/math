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

namespace {

/**
 * A type-triple class type holding return, input/gradient, Hessian types.
 *
 * @tparam T return type
 * @tparam V_t vector type, a *Container*
 * @tparam M_t matrix type
 */
template <typename T, typename V_t, typename M_t>
class func_type_triple {
public:
  func_type_triple() = delete;
  using scalar_t = T;
  using vector_t = V_t;
  using matrix_t = M_t;
};

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
  LineSearchTest() : himmel_f_() {}
  tr_himmelblau_functor<Tr_t> himmel_f_;
};

// types LineSearchTest will be instantiated with + register types
using LineSearchTestTypes = ::testing::Types<
  func_type_triple<float, Eigen::Vector2f, Eigen::Matrix2f>,
  func_type_triple<double, Eigen::Vector2d, Eigen::Matrix2d>,
  func_type_triple<double, pdmath::vector_d, Eigen::Matrix2d>
>;
TYPED_TEST_SUITE(LineSearchTest, LineSearchTestTypes);

}  // namespace
