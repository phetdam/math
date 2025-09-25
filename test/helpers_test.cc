/**
 * @file helpers_test.cc
 * @author Derek Huang
 * @brief Tests for helpers defined in helpers.h
 * @copyright MIT License
 */

#include "pdmath/helpers.h"

#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

#include <Eigen/Core>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/helpers.h"
#include "pdmath/types.h"
#include "pdmath/warnings.h"

/**
 * Values used in the `HelpersVt(One|Two)Test` test fixtures for containers.
 *
 * Used later to test the variadic template versions of `boost_vector_from`,
 * `eigen_vector_from`, `vector_from` to ensure that they work as intended.
 */
#define HELPERS_TEST_VECTOR_VALUES 1., 34, 4.5, 9.82

/**
 * Values used in the `EigenVectorFromMatrixTest` test fixture.
 */
#define HELPERS_TEST_MATRIX_VALUES {1., 34}, {4.5, 9.82}

namespace {

/**
 * Test fixture class for miscellaneous helper tests.
 */
class HelpersMiscTest : public ::testing::Test {
protected:
  // name of exercise example path we want to show
  static const std::string ex_name_;
  // actual full path of exercise example path
  static const std::string ex_path_;
  // frame to display around ex_name_, i.e. a bunch of dashes same length
  static const std::string ex_frame_;
};

PDMATH_STATIC_DEFINE(HelpersMiscTest::ex_name_)("dir1/exampleX.cc");
PDMATH_STATIC_DEFINE(HelpersMiscTest::ex_path_)("math/examples/" + HelpersMiscTest::ex_name_);
PDMATH_STATIC_DEFINE(HelpersMiscTest::ex_frame_)(HelpersMiscTest::ex_name_.size(), '-');

/**
 * Test that `print_example_header` overloads work as expected.
 */
TEST_F(HelpersMiscTest, PrintExampleHeaderTest)
{
  auto header = pdmath::print_example_header(ex_path_, false);
  EXPECT_EQ(header, pdmath::print_example_header(ex_path_.c_str(), false));
  ASSERT_EQ(
    "+" + ex_frame_ + "+\n|" + ex_name_ + "|\n+" + ex_frame_ + "+", header
  );
}

/**
 * Test fixture template for tests requiring a single vector type.
 *
 * For example, testing `boost_vector_from`, `eigen_vector_from`.
 *
 * @tparam V_t *Container* type
 */
template <typename V_t>
class HelpersVtOneTest : public ::testing::Test {
protected:
  static const V_t values_;
};

// as noted before, using {{}} works for both std::vector and Eigen vectors
template <typename V_t>
// MSVC complains (again) about double to float truncation
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
const V_t HelpersVtOneTest<V_t>::values_{{HELPERS_TEST_VECTOR_VALUES}};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER

using HelpersVtOneTestTypes = ::testing::Types<
  pdmath::vector_d,
  std::vector<float>,
  Eigen::VectorXf,
  pdmath::array_d<4>,
  Eigen::Vector4f,
  Eigen::RowVectorXd
>;
TYPED_TEST_SUITE(HelpersVtOneTest, HelpersVtOneTestTypes);

/**
 * Test that `print_vector` overloads work as expected.
 */
TYPED_TEST(HelpersVtOneTest, PrintVectorTest)
{
  // formatted string printed by default
  std::stringstream ss_def;
  for (const auto& v : TestFixture::values_) {
    if (v != *TestFixture::values_.begin()) {
      ss_def << " ";
    }
    ss_def << v;
  }
  // formatted string printed using custom vector_print_policy
  pdmath::vector_print_policy print_policy(",", 2, "[", 2, "]", 2);
  std::stringstream ss_cust;
  ss_cust << "[  ";
  for (const auto& v : TestFixture::values_) {
    if (v != *TestFixture::values_.begin()) {
      ss_cust << ",  ";
    }
    ss_cust << v;
  }
  ss_cust << "  ]";
  EXPECT_EQ(ss_def.str(), pdmath::print_vector(TestFixture::values_, false));
  EXPECT_EQ(
    ss_cust.str(),
    pdmath::print_vector(TestFixture::values_, print_policy, false)
  );
}

/**
 * Test that `boost_vector_from` overloads work as expected.
 */
TYPED_TEST(HelpersVtOneTest, BoostVectorFromTest)
{
  using value_t = typename TypeParam::value_type;
  auto bvec1 = pdmath::boost_vector_from<value_t>(HELPERS_TEST_VECTOR_VALUES);
  auto bvec2 = pdmath::boost_vector_from(TestFixture::values_);
  // boost::numeric::ublas::vector does not define operator==, so use gmock
  // note: we do expect all the values to be equal (no flops are performed)
  EXPECT_THAT(bvec1, ::testing::Pointwise(::testing::Eq(), bvec2));
}

/**
 * Test that `eigen_vector_from` overloads work as expected on vectors.
 */
TYPED_TEST(HelpersVtOneTest, EigenVectorFromTest)
{
  using value_t = typename TypeParam::value_type;
  auto evec1 = pdmath::eigen_vector_from<value_t>(HELPERS_TEST_VECTOR_VALUES);
  auto evec2 = pdmath::eigen_vector_from(TestFixture::values_);
  EXPECT_EQ(evec1, evec2);
}

/**
 * Test that `vector_from` overload using parameter pack works as expected.
 */
TYPED_TEST(HelpersVtOneTest, VectorFromTest)
{
  auto vec = pdmath::vector_from<TypeParam>(HELPERS_TEST_VECTOR_VALUES);
  EXPECT_EQ(vec, TestFixture::values_);
}

/**
 * Test that `unique_vector_from` overload using parameter pack works.
 */
TYPED_TEST(HelpersVtOneTest, UniqueVectorFromTest)
{
  auto vec_p = pdmath::unique_vector_from<TypeParam>(HELPERS_TEST_VECTOR_VALUES);
  EXPECT_EQ(*vec_p, TestFixture::values_);
}

/**
 * Test fixture template for `eigen_vector_from` tests using Eigen matrices.
 *
 * We make this separate from `HelpersVtOneTest` since 2D `Eigen::Matrix`
 * specializations required incompatible nested initialization.
 *
 * @tparam M_t `Eigen::Matrix` specialization
 */
template <typename M_t>
class EigenVectorFromMatrixTest : public ::testing::Test {
protected:
  static const M_t values_;
};

template <typename M_t>
// MSVC complains (again) about double to float truncation
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
const M_t EigenVectorFromMatrixTest<M_t>::values_{HELPERS_TEST_MATRIX_VALUES};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER

using EigenVectorFromMatrixTestTypes = ::testing::Types<
  Eigen::Matrix2f, Eigen::Matrix2Xd, Eigen::MatrixXf
>;
TYPED_TEST_SUITE(EigenVectorFromMatrixTest, EigenVectorFromMatrixTestTypes);

/**
 * Test that `eigen_vector_from` also works with Eigen matrix typedefs.
 *
 * Also checks that the `eigen_vector_from` overload taking an
 * `Eigen::MatrixBase<Derived>` works as expected on expressions.
 */
TYPED_TEST(EigenVectorFromMatrixTest, Test)
{
  using value_t = typename TypeParam::value_type;
  auto evec1 = pdmath::eigen_vector_from<value_t>(HELPERS_TEST_VECTOR_VALUES);
  // canceling expression resolves to TestFixture::values_
  auto evec2 = pdmath::eigen_vector_from(
    TestFixture::values_ - TestFixture::values_ + TestFixture::values_
  );
  EXPECT_EQ(evec1, evec2);
}

/**
 * Test fixture template for tests requiring two vector types.
 *
 * @tparam Tp_t `std::pair<T, U>` with `T`, `U` *Container* types
 */
template <typename Tp_t>
class HelpersVtTwoTest : public ::testing::Test {
protected:
  using V1_t = typename Tp_t::first_type;
  using V2_t = typename Tp_t::second_type;
  // element types must of course be the same
  static_assert(
    std::is_same_v<typename V1_t::value_type, typename V2_t::value_type>,
    "V1_t::value_type, V2_t::value_type must be the same"
  );
  static const V1_t values_1_;
  static const V2_t values_2_;
};

template <typename Tp_t>
// MSVC complains (again) about double to float truncation
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
const typename Tp_t::first_type HelpersVtTwoTest<Tp_t>::values_1_{
  {HELPERS_TEST_VECTOR_VALUES}
};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
template <typename Tp_t>
// MSVC complains (again) about double to float truncation
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
const typename Tp_t::second_type HelpersVtTwoTest<Tp_t>::values_2_{
  {HELPERS_TEST_VECTOR_VALUES}
};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER

using HelpersVtTwoTestTypes = ::testing::Types<
  std::pair<pdmath::vector_d, Eigen::VectorXd>,
  std::pair<Eigen::Vector4f, pdmath::vector_f>,
  std::pair<Eigen::RowVectorXd, Eigen::Vector4d>,
  std::pair<pdmath::array_f<4>, Eigen::VectorXf>,
  std::pair<pdmath::vector_d, pdmath::array_d<4>>,
  std::pair<pdmath::array_f<4>, Eigen::RowVector4f>,
  std::pair<Eigen::RowVector4d, pdmath::vector_d>
>;
TYPED_TEST_SUITE(HelpersVtTwoTest, HelpersVtTwoTestTypes);

/**
 * Macro for `using`-declarations for `HelpersVtTwoTest` `TypeParam` types.
 *
 * Saves us from typing `typename TypeParam::(first|second)_type` or
 * `typename TestFixture::V[12]_t` repeatedly.
 */
#define HelpersVtTwoTest_TYPES_USING_DECLS \
  using V1_t = typename TypeParam::first_type; \
  using V2_t = typename TypeParam::second_type;

/**
 * Test that `vector_from` overload using a *Container* works as expected.
 */
TYPED_TEST(HelpersVtTwoTest, VectorFromTest)
{
  HelpersVtTwoTest_TYPES_USING_DECLS
  auto vec1 = pdmath::vector_from<V1_t>(TestFixture::values_2_);
  auto vec2 = pdmath::vector_from<V2_t>(TestFixture::values_1_);
  EXPECT_EQ(vec1, TestFixture::values_1_);
  EXPECT_EQ(vec2, TestFixture::values_2_);
}

/**
 * Test that `unique_vector_from` overload using a *Container* works.
 */
TYPED_TEST(HelpersVtTwoTest, UniqueVectorFromTest)
{
  HelpersVtTwoTest_TYPES_USING_DECLS
  auto vec1_p = pdmath::unique_vector_from<V1_t>(TestFixture::values_2_);
  auto vec2_p = pdmath::unique_vector_from<V2_t>(TestFixture::values_1_);
  EXPECT_EQ(*vec1_p, TestFixture::values_1_);
  EXPECT_EQ(*vec2_p, TestFixture::values_2_);
}

}  // namespace
