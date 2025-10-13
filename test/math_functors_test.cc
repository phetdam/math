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
#include <gtest/gtest.h>

#include "pdmath/helpers.h"
#include "pdmath/types.h"
#include "pdmath/warnings.h"
#include "pdmath/testing/macros.h"
#include "pdmath/testing/utils.h"

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
 * Templated test fixture for testing the `quadratic_functor` template class.
 *
 * @tparam Tp_t `func_type_pair` specialization
 */
template <typename Tp_t>
class QuadraticFunctorTest
  : public ::testing::Test,
    public pdmath::testing::tol_mixin<typename Tp_t::scalar_type> {
protected:
  // convenience type aliases
  using gradient_type = typename Tp_t::gradient_type;
  using hessian_type = typename Tp_t::hessian_type;
  // shared pointers to functor Hessian and affine terms. for some reason,
  // using parentheses results in a compiler error, but list-init is fine.
  static inline const std::shared_ptr<hessian_type> hess_{
    new hessian_type{EIGEN_HESS_INIT}
  };
  static inline const std::shared_ptr<gradient_type> aff_{
    pdmath::unique_vector_from<gradient_type>(AFF_TERMS_INIT)
  };
  // minimizer of the quadratic with hess_d_/hess_f_, aff_, shf_
  static inline const gradient_type sol_{
    pdmath::vector_from<gradient_type>(
      hess_->householderQr().solve(-pdmath::eigen_vector_from(*aff_)).eval()
    )
  };
// MSVC warns about truncating from double to const T
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
  // offset for the quadratic functor
  static constexpr typename Tp_t::scalar_type shf_ = 0.7;
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
  // quadratic functor we are interested in
  static inline pdmath::quadratic_functor<gradient_type, hessian_type> quad_{
    hess_, aff_, shf_
  };
  // vector of 3 zeros that will prove useful later. note that using list-init
  // works for both Eigen::Matrix specializations and std:;vector, as
  // {{0, 0, 0,}} is interpreted by std::vector as calling the init list ctor.
  static inline const gradient_type zeros_3_{{0., 0., 0.}};
};

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
  // since we have a templated test that could have different types
  EXPECT_THAT(this->quad_.d1(this->sol_), this->AllNearZero());
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
  : public ::testing::Test,
    public pdmath::testing::tol_mixin<typename Tp_t::scalar_type> {
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
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
const typename Tp_t::gradient_type HimmelblauFunctorTest<Tp_t>::min_2_{
  {HIMMELBLAU_ZERO_2}
};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
template <typename Tp_t>
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
const typename Tp_t::gradient_type HimmelblauFunctorTest<Tp_t>::min_3_{
  {HIMMELBLAU_ZERO_3}
};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
template <typename Tp_t>
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
const typename Tp_t::gradient_type HimmelblauFunctorTest<Tp_t>::min_4_{
  {HIMMELBLAU_ZERO_4}
};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
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

/**
 * Macro to reduce boilerplate of checking `himmelblau_functor` gradient zeros.
 *
 * @note Since the Himmelblau functor gradients tend to be rather coarse, we
 *     use `pdmath::testing::tol_mixin<float>::tol` for the tolerance.
 *
 * @param zero a `Tp_t::gradient_type` zero candidate
 */
#define HimmelblauFunctorTest_EXPECT_GRAD_NEAR_ZERO(zero) \
  EXPECT_THAT( \
    this->himmel_.d1(TestFixture::zero), \
    pdmath \
    ::testing \
    ::match_all_near_zero(pdmath::testing::tol_mixin<float>::tol()) \
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

/**
 * Macro to reduce boilerplate checking if `himmelblau_functor` Hessians PSD.
 *
 * A positive semidefinite matrix necessarily has nonnegative determinant.
 *
 * @param zero a `Tp_t::gradient_type` zero candidate
 */
#define HimmelblauFunctorTest_EXPECT_HESSIAN_NEAR_PSD(zero) \
  EXPECT_GE(this->himmel_.d2(TestFixture::zero).determinant(), 0)

/**
 * Test that the `himmelblau_functor` Hessians are PSD at the zeros.
 */
TYPED_TEST(HimmelblauFunctorTest, PSDHessTest)
{
  HimmelblauFunctorTest_EXPECT_HESSIAN_NEAR_PSD(min_1_);
  HimmelblauFunctorTest_EXPECT_HESSIAN_NEAR_PSD(min_2_);
  HimmelblauFunctorTest_EXPECT_HESSIAN_NEAR_PSD(min_3_);
  HimmelblauFunctorTest_EXPECT_HESSIAN_NEAR_PSD(min_4_);
}

}  // namespace
