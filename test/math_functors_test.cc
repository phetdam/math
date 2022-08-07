/**
 * @file math_functors_test.cc
 * @author Derek Huang
 * @brief Tests for the templated math functors in `math_functors.h`
 * @copyright MIT License
 */

#include "pdmath/math_functors.h"

#include <memory>
#include <vector>

#include <Eigen/Core>
#include <Eigen/QR>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/helpers.h"
#include "pdmath/types.h"

namespace {

/**
 * Comma-separated initializers for initializing `hess_d_` and `hess_f_`.
 *
 * This defines a positive definite 3x3 matrix (from Wikipedia).
 */
#define EIGEN_HESS_INIT {2, -1, 0}, {-1, 2, -1}, {0, -1, 2}

/**
 * Comma-separate list for initializing `aff_`.
 *
 * Defines affine terms of a 3D quadratic function.
 */
#define AFF_TERMS_INIT 0.4, -0.2, 1

/**
 * Templated `using` definition for `quadratic_functor`.
 *
 * @tparam T scalar type
 * @tparam M_t matrix type with scalar type `T`, ex. an Eigen matrix
 */
template <typename T, typename M_t>
using qf_t = pdmath::quadratic_functor<T, std::vector<T>, M_t>;

/**
 * Templated test fixture for math functors tests.
 */
template <typename T>
class MathFunctorsTest : public ::testing::Test {
protected:
  /**
   * Default constructor.
   *
   * Her we initialize the solution vector `sol_` and the functors.
   */
  MathFunctorsTest()
    : hess_d_(new Eigen::MatrixX<T>{EIGEN_HESS_INIT}),
      hess_f_(new Eigen::Matrix3<T>{EIGEN_HESS_INIT}),
      aff_(new std::vector<T>{AFF_TERMS_INIT}),
      sol_(hess_f_->householderQr().solve(-pdmath::eigen_vector_from(*aff_))),
      quad_d_(hess_d_, aff_, shf_),
      quad_f_(hess_f_, aff_, shf_)
  {}

  /**
   * Setup method for the `MathFunctorsTest`.
   *
   * We test correctness of `sol_` here since `ASSERT_*` can't be used in ctor.
   */
  void SetUp() override
  {
    // need to use Eigen vector instead of STL vector
    Eigen::Vector3<T> aff{AFF_TERMS_INIT};
    // shouldn't matter if we use hess_f_ or hess_d_
    ASSERT_EQ(sol_, hess_d_->householderQr().solve(-aff));
  }

  // try using both Eigen::Dynamic and fixed-at-compile time matrices. had some
  // issues using std::make_shared so we opt to directly initialize
  const std::shared_ptr<Eigen::MatrixX<T>> hess_d_;
  const std::shared_ptr<Eigen::Matrix3<T>> hess_f_;
  // purposely mix std::vector<T> to test how general the functors are
  const std::shared_ptr<std::vector<T>> aff_;
  // minimizer of the quadratic with hess_d_/hess_f_, aff_, shf_. this cannot
  // be computed at compile time (Eigen objects are not constexpr), so we
  // compute this in the constructor but do ASSERT_EQ check in SetUp, as we
  // are not allowed to call ASSERT_* macros in ctor/dtor of ::testing::Test.
  const Eigen::Vector3<T> sol_;
  // the quadratic functors we are interested in (fixed and dynamic matrices)
  qf_t<T, Eigen::MatrixX<T>> quad_d_;
  qf_t<T, Eigen::Matrix3<T>> quad_f_;

  // offset for the quadratic functor
  static constexpr T shf_ = 0.7;
};

using MathFunctorsTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(MathFunctorsTest, MathFunctorsTypes);

/**
 * Test that the `quadratic_functor` evaluates correctly.
 *
 * We first check that the functors evaluate correctly at
 */
TYPED_TEST(MathFunctorsTest, QFZeroTest)
{
  const std::vector<TypeParam> zeros(3, 0);
  EXPECT_DOUBLE_EQ(TestFixture::shf_, this->quad_d_(zeros));
  EXPECT_DOUBLE_EQ(TestFixture::shf_, this->quad_f_(zeros));
}

/**
 * Test that the `quadratic_functor` gradient zeros correctly.
 *
 * That is, at the minimum `sol_`, it should be close to zero.
 */
TYPED_TEST(MathFunctorsTest, QuadraticFunctorGradZeroTest)
{
  // sol_ is an Eigen::Vector3, so convert to std::vector
  const std::vector<TypeParam> sol(this->sol_.cbegin(), this->sol_.cend());
  // can't use the Google Test ::testing::Each(::testing::FloatEq(0.)) here
  // since we have a templated test that could have different types. therefore,
  // to allow boxing, we using ::testing::Ge and ::testing::Le. since we are
  // using single-precision floats in some cases, we make tol a bit above the
  // std::numeric_limits<float>::epsilon() value, 1.19209e-07.
  const double ftol = 1e-6;
  const auto all_near_zero = ::testing::Each(
    ::testing::AllOf(::testing::Ge(-ftol), ::testing::Le(ftol))
  );
  EXPECT_THAT(this->quad_d_.d1(sol), all_near_zero);
  EXPECT_THAT(this->quad_f_.d1(sol), all_near_zero);
}

}  // namespace
