/**
 * @file math_functors_test.cc
 * @author Derek Huang
 * @brief Tests for the templated math functors in `math_functors.h`
 * @copyright MIT License
 */

#include "pdmath/math_functors.h"

#include <array>
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
 * Templated mixin class providing a templated comparison tolerance function.
 *
 * Mostly just provides the float loose comparison tolerance we use a lot.
 */
template <typename T>
class TolMixin {
public:
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
float TolMixin<float>::tol() { return 1e-4f; }

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
 * @tparam Tp_t `func_type_pair` specialization
 */
template <typename Tp_t>
class QuadraticFunctorTest
  : public ::testing::Test, public TolMixin<typename Tp_t::scalar_type> {
protected:
  // convenience type aliases
  using gradient_type = typename Tp_t::gradient_type;
  using hessian_type = typename Tp_t::hessian_type;

  /**
   * Default constructor.
   *
   * Here we initialize the solution vector `sol_` and the functors.
   */
  QuadraticFunctorTest()
    : hess_(new hessian_type{EIGEN_HESS_INIT}),
      aff_(pdmath::unique_vector_from<gradient_type>(AFF_TERMS_INIT)),
      sol_(
        pdmath::vector_from<gradient_type>(
          hess_->householderQr().solve(-pdmath::eigen_vector_from(*aff_)).eval())),
      quad_(hess_, aff_, shf_),
      // method of construction works for both `Eigen::Matrix` specializations
      // and `std::vector`, as `{{0, 0, 0}}` is interpreted by
      // list-initialization constructor taking a `std::initializer_list<T>`.
      zeros_3_{{0., 0., 0.}}
  {}

  // shared pointers to functor Hessian and affine terms
  const std::shared_ptr<hessian_type> hess_;
  const std::shared_ptr<gradient_type> aff_;
  // minimizer of the quadratic with hess_d_/hess_f_, aff_, shf_. this cannot
  // be computed at compile time (Eigen objects are not constexpr), so we
  // compute this in the constructor but do ASSERT_EQ check in SetUp, as we
  // are not allowed to call ASSERT_* macros in ctor/dtor of ::testing::Test.
  const gradient_type sol_;
  // quadratic functor we are interested in
  pdmath::quadratic_functor<gradient_type, hessian_type> quad_;
  // vector of 3 zeros that will prove useful later
  const gradient_type zeros_3_;

// MSVC warns about truncating from double to const T
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4305)
#endif  // _MSC_VER
  // offset for the quadratic functor
  static constexpr typename Tp_t::scalar_type shf_ = 0.7;
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
};

#undef EIGEN_HESS_INIT
#undef AFF_TERMS_INIT

// types used for QuadraticFunctorTest, mixing STL and Eigen types
using QuadraticFunctorTypes = ::testing::Types<
  pdmath::testing::func_type_pair<pdmath::vector_d, Eigen::Matrix3d>,
  pdmath::testing::func_type_pair<Eigen::Vector3f, Eigen::MatrixXf>,
  pdmath::testing::func_type_pair<Eigen::VectorXd, Eigen::Matrix3d>,
  pdmath::testing::func_type_pair<pdmath::vector_f, Eigen::Matrix3f>,
  pdmath::testing::func_type_pair<pdmath::array_triple_d, Eigen::MatrixXd>
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
  // can't use the Google Test ::testing::Each(::testing::FloatEq(0.)) here
  // since we have a templated test that could have different types. therefore,
  // to allow boxing, we using ::testing::Ge and ::testing::Le.
  const auto all_near_zero = ::testing::Each(
    ::testing::AllOf(
      ::testing::Ge(-TestFixture::tol()), ::testing::Le(TestFixture::tol())
    )
  );
  EXPECT_THAT(this->quad_.d1(this->sol_), all_near_zero);
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
 * Templated test fixture for testing the `himmelblau_functor`.
 *
 * @tparam Tp_t `func_type_pair` specialization
 */
template <typename Tp_t>
class HimmelblauFunctorTest
  : public ::testing::Test, public TolMixin<typename Tp_t::scalar_type> {
protected:
  // convenience type aliases
  using gradient_type = typename Tp_t::gradient_type;
  using hessian_type = typename Tp_t::hessian_type;

protected:
  HimmelblauFunctorTest() : himmel_() {}

  // Himmelblau functor we are interested in
  pdmath::himmelblau_functor<gradient_type, hessian_type> himmel_;
  // Himmelblau's function has 4 minimizers where the function is zero
  static const gradient_type min_1_;
  static const gradient_type min_2_;
  static const gradient_type min_3_;
  static const gradient_type min_4_;
};

// initialize all four zeros of the Himmelblau function
template <typename Tp_t>
const typename Tp_t::gradient_type HimmelblauFunctorTest<Tp_t>::min_1_{
  {HIMMELBLAU_ZERO_1}
};
template <typename Tp_t>
// MSVC warns about truncation from double to const T
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4305)
#endif  // _MSC_VER
const typename Tp_t::gradient_type HimmelblauFunctorTest<Tp_t>::min_2_{
  {HIMMELBLAU_ZERO_2}
};
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
template <typename Tp_t>
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4305)
#endif  // _MSC_VER
const typename Tp_t::gradient_type HimmelblauFunctorTest<Tp_t>::min_3_{
  {HIMMELBLAU_ZERO_3}
};
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
template <typename Tp_t>
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4305)
#endif  // _MSC_VER
const typename Tp_t::gradient_type HimmelblauFunctorTest<Tp_t>::min_4_{
  {HIMMELBLAU_ZERO_4}
};
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER

using HimmelblauFunctorTestTypes = ::testing::Types<
  pdmath::testing::func_type_pair<pdmath::array_pair_d, Eigen::Matrix2d>,
  pdmath::testing::func_type_pair<pdmath::vector_f, Eigen::MatrixXf>,
  pdmath::testing::func_type_pair<Eigen::Vector2d, Eigen::MatrixXd>,
  pdmath::testing::func_type_pair<Eigen::VectorXf, Eigen::Matrix2Xf>
>;
TYPED_TEST_SUITE(HimmelblauFunctorTest, HimmelblauFunctorTestTypes);

/**
 * Macro to reduce boilerplate of checking `himmelblau_functor` zeros.
 *
 * @param zero a `Tp_t::gradient_type` zero candidate
 */
#define HimmelblauFunctorTest_EXPECT_NEAR_ZERO(zero) \
  EXPECT_NEAR(0, this->himmel_(TestFixture::zero), TestFixture::tol())

/**
 * Test that the `himmelblau_functor` zeros correctly.
 */
TYPED_TEST(HimmelblauFunctorTest, ZeroEvalTest)
{
  HimmelblauFunctorTest_EXPECT_NEAR_ZERO(min_1_);
  HimmelblauFunctorTest_EXPECT_NEAR_ZERO(min_2_);
  HimmelblauFunctorTest_EXPECT_NEAR_ZERO(min_3_);
  HimmelblauFunctorTest_EXPECT_NEAR_ZERO(min_4_);
}

#undef HimmelblauFunctorTest_EXPECT_NEAR_ZERO

/**
 * Macro to reduce boilerplate of checking `himmelblau_functor` gradient zeros.
 *
 * We need to use `vector_from<Eigen::VectorX<typename TypeParam::scalar_type>>`
 * to ensure that the result of `himmel_.d1` is a `Eigen::VectorX`
 * specialization so we can use the `isZero` method.
 *
 * @note Since the Himmelblau functor gradients tend to be rather coarse, we
 *     use `TolMixin<float>::tol` explicitly to get the comparison tolerance.
 *
 * @param zero a `Tp_t::gradient_type` zero candidate
 */
#define HimmelblauFunctorTest_EXPECT_GRAD_NEAR_ZERO(zero) \
  EXPECT_TRUE( \
    pdmath::vector_from<Eigen::VectorX<typename TypeParam::scalar_type>>( \
      this->himmel_.d1(TestFixture::zero)) \
        .isZero(TolMixin<float>::tol()) \
  )

/**
 * Test that the `himmelblau_functor` gradients are near zero at the zeros.
 */
TYPED_TEST(HimmelblauFunctorTest, GradNearZeroTest)
{
  HimmelblauFunctorTest_EXPECT_GRAD_NEAR_ZERO(min_1_);
  HimmelblauFunctorTest_EXPECT_GRAD_NEAR_ZERO(min_2_);
  HimmelblauFunctorTest_EXPECT_GRAD_NEAR_ZERO(min_3_);
  HimmelblauFunctorTest_EXPECT_GRAD_NEAR_ZERO(min_4_);
}

#undef HimmelblauFunctorTest_EXPECT_GRAD_NEAR_ZERO

/**
 * Macro to reduce boilerplate checking if `himmelblau_functor` Hessians PSD.
 *
 * A positive semidefinite matrix necessarily has nonnegative determinant.
 *
 * @param zero a `Tp_t::gradient_type` zero candidate
 */
#define HimmelblauFunctortest_EXPECT_HESSIAN_NEAR_PSD(zero) \
  EXPECT_GE(this->himmel_.d2(TestFixture::zero).determinant(), 0)

/**
 * Test that the `himmelblau_functor` Hessians are PSD at the zeros.
 */
TYPED_TEST(HimmelblauFunctorTest, PSDHessTest)
{
  HimmelblauFunctortest_EXPECT_HESSIAN_NEAR_PSD(min_1_);
  HimmelblauFunctortest_EXPECT_HESSIAN_NEAR_PSD(min_2_);
  HimmelblauFunctortest_EXPECT_HESSIAN_NEAR_PSD(min_3_);
  HimmelblauFunctortest_EXPECT_HESSIAN_NEAR_PSD(min_4_);
}

#undef HimmelblauFunctortest_EXPECT_HESSIAN_NEAR_PSD

}  // namespace
