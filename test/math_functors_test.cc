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
#include <Eigen/LU>
#include <Eigen/QR>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/helpers.h"
#include "pdmath/types.h"
#include "pdmath/testing/macros.h"
#include "pdmath/testing/utils.h"

namespace {

/**
 * Templated base test fixture for testing math functors.
 *
 * Mostly just provides the float loose comparison tolerance we use a lot.
 */
template <typename T>
class MathFunctorsTestBase : public ::testing::Test {
protected:
// MSVC warns about truncation from double to const T
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4305)
#endif  // _MSC_VER
  // float loose comparison tolerance. since we are using single-precision
  // floats in some cases, we make tol a bit below the square root of the
  // std::numeric_limits<float>::epsilon() value, 1.19209e-07.
  static constexpr T ftol_ = 1e-4;
  // double loose comparison tolerance. for comparison, the
  // std::numeric_limits<double>::epsilon() value is 2.22045e-16.
  static constexpr T dtol_ = 1e-8;
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER

  /**
   * Return comparison tolerance for doubles.
   *
   * @note `std::numeric_limits<double>::epsilon()` returns `2.22045e-16`.
   */
  static T tol() { return 1e-8; }
};

/**
 * Return looser comparison tolerance for floats.
 *
 * @note `std::numeric_limits<float>::epsilon()` returns `1.19209e-07`.
 */
template <>
float MathFunctorsTestBase<float>::tol() { return 1e-4f; }

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
 * Templated test fixture for testing the `quadratic_functor` template class.
 *
 * @tparam Tr_t `func_type_triple` specialization
 */
template <typename Tr_t>
class QuadraticFunctorTest :
  public MathFunctorsTestBase<typename Tr_t::scalar_t> {
protected:
  // using declarations for the Tr_t types
  using T = typename Tr_t::scalar_t;
  using V_t = typename Tr_t::vector_t;
  using M_t = typename Tr_t::matrix_t;

  /**
   * Default constructor.
   *
   * Here we initialize the solution vector `sol_` and the functors.
   */
  QuadraticFunctorTest()
    : hess_(new M_t{EIGEN_HESS_INIT}),
      aff_(pdmath::unique_vector_from<V_t>(AFF_TERMS_INIT)),
      sol_(
        pdmath::vector_from<V_t>(
          hess_->householderQr().solve(-pdmath::eigen_vector_from(*aff_)).eval())),
      quad_(hess_, aff_, shf_),
      // method of construction works for both `Eigen::Matrix` specializations
      // and `std::vector`, as `{{0, 0, 0}}` is interpreted by
      // list-initialization constructor taking a `std::initializer_list<T>`.
      zeros_3_{{0., 0., 0.}}
  {}

  // shared pointers to functor Hessian and affine terms
  const std::shared_ptr<M_t> hess_;
  const std::shared_ptr<V_t> aff_;
  // minimizer of the quadratic with hess_d_/hess_f_, aff_, shf_. this cannot
  // be computed at compile time (Eigen objects are not constexpr), so we
  // compute this in the constructor but do ASSERT_EQ check in SetUp, as we
  // are not allowed to call ASSERT_* macros in ctor/dtor of ::testing::Test.
  const V_t sol_;
  // quadratic functor we are interested in
  pdmath::quadratic_functor<T, V_t, M_t> quad_;
  // vector of 3 zeros that will prove useful later
  const V_t zeros_3_;

// MSVC warns about truncating from double to const T
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4305)
#endif  // _MSC_VER
  // offset for the quadratic functor
  static constexpr T shf_ = 0.7;
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
};

#undef EIGEN_HESS_INIT
#undef AFF_TERMS_INIT

// types used for QuadraticFunctorTest, mixing STL and Eigen types
using QuadraticFunctorTypes = ::testing::Types<
  pdmath::testing::func_type_triple<double, pdmath::double_vector, Eigen::Matrix3d>,
  pdmath::testing::func_type_triple<float, Eigen::Vector3f, Eigen::MatrixXf>,
  pdmath::testing::func_type_triple<double, Eigen::VectorXd, Eigen::Matrix3d>,
  pdmath::testing::func_type_triple<float, pdmath::float_vector, Eigen::Matrix3f>
>;
TYPED_TEST_SUITE(QuadraticFunctorTest, QuadraticFunctorTypes);

/**
 * Test that the `quadratic_functor` evaluates correctly.
 *
 * We first check that the functors evaluate correctly at zero.
 */
TYPED_TEST(QuadraticFunctorTest, ZeroEvalTest)
{
  EXPECT_DOUBLE_EQ(TestFixture::shf_, this->quad_(this->zeros_3_));
}

/**
 * Test that the `quadratic_functor` gradient zeros correctly.
 *
 * That is, at the minimum `sol_`, it should be close to zero.
 */
TYPED_TEST(QuadraticFunctorTest, GradNearZeroTest)
{
  // sol_ is an Eigen::Vector3, so convert to std::vector
  // const std::vector<TypeParam> sol(this->sol_.cbegin(), this->sol_.cend());
  // can't use the Google Test ::testing::Each(::testing::FloatEq(0.)) here
  // since we have a templated test that could have different types. therefore,
  // to allow boxing, we using ::testing::Ge and ::testing::Le.
  const auto all_near_zero = ::testing::Each(
    ::testing::AllOf(
      ::testing::Ge(-TestFixture::tol()), ::testing::Le(TestFixture::tol())
    )
  );
  EXPECT_THAT(this->quad_.d1(this->sol_), all_near_zero);
  // EXPECT_THAT(this->quad_f_.d1(sol), all_near_zero);
}

/**
 * Test that both `quadratic_functor` Hessians are equal.
 */
TYPED_TEST(QuadraticFunctorTest, EqualHessianTest)
{
  // does not matter what vector we pass to d2
  EXPECT_EQ(this->quad_.d2(this->zeros_3_), this->quad_.d2(*this->aff_));
}

/**
 * Templated `using` definition for `himmelblau_functor`.
 *
 * @tparam T scalar type
 * @tparam M_t matrix type with scalar type `T`, ex. an Eigen matrix
 */
template <typename T, typename M_t>
using hf_t = pdmath::himmelblau_functor<T, Eigen::Vector2<T>, M_t>;

/**
 * Templated test fixture for testing the `himmelblau_functor`.
 */
template <typename T>
class HimmelblauFunctorTest : public MathFunctorsTestBase<T> {
protected:
  /**
   * Default constructor.
   *
   * Here we initialize the Himmelblau function minimizers.
   */
  HimmelblauFunctorTest()
    : min_1_(new Eigen::Vector2<T>{HIMMELBLAU_ZERO_1}),
      min_2_(new Eigen::Vector2<T>{HIMMELBLAU_ZERO_2}),
      min_3_(new Eigen::Vector2<T>{HIMMELBLAU_ZERO_3}),
      min_4_(new Eigen::Vector2<T>{HIMMELBLAU_ZERO_1}),
      himmel_d_(),
      himmel_f_()
  {}

  // Himmelblau's function has 4 minimizers where the function is zero
  const std::unique_ptr<Eigen::Vector2<T>> min_1_;
  const std::unique_ptr<Eigen::Vector2<T>> min_2_;
  const std::unique_ptr<Eigen::Vector2<T>> min_3_;
  const std::unique_ptr<Eigen::Vector2<T>> min_4_;
  // Himmelblau functors with different Eigen matrix types
  hf_t<T, Eigen::MatrixX<T>> himmel_d_;
  hf_t<T, Eigen::Matrix2<T>> himmel_f_;
};

// TODO: use same types as QuadraticFunctorTypes
using MathFunctorsTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(HimmelblauFunctorTest, MathFunctorsTypes);

/**
 * Macro to reduce boilerplate of checking `himmelblau_functor` zeros.
 *
 * @param name Name of `hf_t<T, M_t>` member of `HimmelblauFunctorTest`
 */
#define HimmelBlauFunctorTest_EXPECT_NEAR_ZEROS(name) \
  EXPECT_NEAR(0, this->name(*this->min_1_), TestFixture::dtol_); \
  EXPECT_NEAR(0, this->name(*this->min_2_), TestFixture::dtol_); \
  EXPECT_NEAR(0, this->name(*this->min_3_), TestFixture::dtol_); \
  EXPECT_NEAR(0, this->name(*this->min_4_), TestFixture::dtol_)

/**
 * Test that the `himmelblau_functor` zeros correctly.
 */
TYPED_TEST(HimmelblauFunctorTest, ZeroEvalTest)
{
  HimmelBlauFunctorTest_EXPECT_NEAR_ZEROS(himmel_d_);
  HimmelBlauFunctorTest_EXPECT_NEAR_ZEROS(himmel_f_);
}

#undef HimmelBlauFunctorTest_EXPECT_NEAR_ZEROS

/**
 * Macro to reduce boilerplate of checking `himmelblau_functor` gradient zeros.
 *
 * Since we use `Eigen::Vector2<T>` in `hf_t<T, M_t>`, we can use the existing
 *`isZero` method to check if gradients are close to zero.
 *
 * @param name Name of `hf_t<T, M_t>` member of `HimmelblauFunctorTest`
 */
#define HimmelblauFunctorTest_EXPECT_GRADS_NEAR_ZERO(name) \
  EXPECT_TRUE(this->name.d1(*this->min_1_).isZero(TestFixture::ftol_)); \
  EXPECT_TRUE(this->name.d1(*this->min_2_).isZero(TestFixture::ftol_)); \
  EXPECT_TRUE(this->name.d1(*this->min_3_).isZero(TestFixture::ftol_)); \
  EXPECT_TRUE(this->name.d1(*this->min_4_).isZero(TestFixture::ftol_))

/**
 * Test that the `himmelblau_functor` gradients are near zero at the zeros.
 */
TYPED_TEST(HimmelblauFunctorTest, GradNearZeroTest)
{
  HimmelblauFunctorTest_EXPECT_GRADS_NEAR_ZERO(himmel_d_);
  HimmelblauFunctorTest_EXPECT_GRADS_NEAR_ZERO(himmel_f_);  //sds
}

#undef HimmelblauFunctorTest_EXPECT_GRADS_NEAR_ZERO

/**
 * Macro to reduce boilerplate checking if `himmelblaue_functor` Hessians PSD.
 *
 * A positive semidefinite matrix necessarily has nonnegative determinant.
 *
 * @param name Name of `hf_t<T, M_t>` member of `HimmelblauFunctorTest`
 */
#define HimmelblauFunctortest_EXPECT_HESSIANS_NEAR_PSD(name) \
  EXPECT_GE(this->name.d2(*this->min_1_).determinant(), 0); \
  EXPECT_GE(this->name.d2(*this->min_2_).determinant(), 0); \
  EXPECT_GE(this->name.d2(*this->min_3_).determinant(), 0); \
  EXPECT_GE(this->name.d2(*this->min_4_).determinant(), 0)

/**
 * Test that the `himmelblau_functor` Hessians are PSD at the zeros.
 */
TYPED_TEST(HimmelblauFunctorTest, PSDHessTest)
{
  HimmelblauFunctortest_EXPECT_HESSIANS_NEAR_PSD(himmel_d_);
  HimmelblauFunctortest_EXPECT_HESSIANS_NEAR_PSD(himmel_f_);
}

#undef HimmelblauFunctortest_EXPECT_HESSIANS_NEAR_PSD

}  // namespace
