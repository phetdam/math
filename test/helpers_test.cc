/**
 * @file helpers_test.cc
 * @author Derek Huang
 * @brief Tests for helpers defined in helpers.h
 * @copyright MIT License
 */

#include "pdmath/helpers.h"

#include <sstream>
#include <type_traits>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/features.h"
// TODO: remove range_format.h include once we migrate tests + fixtures
#include "pdmath/range_format.h"
#include "pdmath/testing/utils.h"
#include "pdmath/types.h"
#include "pdmath/warnings.h"

#if PDMATH_HAS_EIGEN3
#include <Eigen/Core>
#endif  // PDMATH_HAS_EIGEN3

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
  static inline const std::string ex_name_{"dir1/exampleX.cc"};
  // actual full path of exercise example path
  static inline const std::string ex_path_{"math/examples/" + ex_name_};
  // frame to display around ex_name_, i.e. a bunch of dashes same length
  // note: not defined inline since list-init would result in selection of an
  // unwanted overload (the initializer_list<char> overload)
  static const std::string ex_frame_;
};

// note: see above for why we don't define this inline
PDMATH_STATIC_DEFINE(HelpersMiscTest::ex_frame_)
  (HelpersMiscTest::ex_name_.size() + 2u, '-');

/**
 * Test that `print_example_header` works as expected.
 */
TEST_F(HelpersMiscTest, PrintExampleHeaderTest)
{
  auto header = pdmath::print_example_header(ex_path_, false);
  ASSERT_EQ(
    "+" + ex_frame_ + "+\n| " + ex_name_ + " |\n+" + ex_frame_ + "+", header
  );
}

/**
 * Test fixture template for tests requiring a single vector type.
 *
 * For example, testing `boost_vector_from`, `eigen_vector_from`.
 *
 * @tparam T *Container* type
 */
template <typename T>
class HelpersVtOneTest : public ::testing::Test {
protected:
  // as noted before, using {{}} works for both std::vector and Eigen vectors
  // MSVC complains (again) about double to float truncation
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4305)
  static inline const T values_{{HELPERS_TEST_VECTOR_VALUES}};
PDMATH_MSVC_WARNINGS_POP()

  /**
   * `RangeFormatTest` implementation.
   */
  void RangeFormatTest() const
  {
    // formatted string printed by default
    std::stringstream ss_def;
    for (const auto& v : values_) {
      if (v != *values_.begin()) {
        ss_def << " ";
      }
      ss_def << v;
    }
    // formatted string printed using custom formatting policy
    pdmath::range_format_policy policy(",", 2, "[", 2, "]", 2);
    std::stringstream ss_cust;
    ss_cust << "[  ";
    for (const auto& v : values_) {
      if (v != *values_.begin()) {
        ss_cust << ",  ";
      }
      ss_cust << v;
    }
    ss_cust << "  ]";
    EXPECT_EQ(ss_def.str(), pdmath::to_string(values_));
    EXPECT_EQ(ss_cust.str(), pdmath::to_string(values_, policy));
  }

#if PDMATH_HAS_BOOST
  /**
   * `BoostVectorFromTest` implementation.
   */
  void BoostVectorFromTest() const
  {
    // note: not all containers will have a value_type type member
    using value_t = typename T::value_type;
    auto bvec1 = pdmath::boost_vector_from<value_t>(HELPERS_TEST_VECTOR_VALUES);
    auto bvec2 = pdmath::boost_vector_from(values_);
    // boost::numeric::ublas::vector does not define operator==, so use gmock
    // note: we do expect all the values to be equal (no flops are performed)
    EXPECT_THAT(bvec1, ::testing::Pointwise(::testing::Eq(), bvec2));
  }
#endif  // !PDMATH_HAS_BOOST

#if PDMATH_HAS_EIGEN3
  /**
   * `EigenVectorFromTest` implementation.
   */
  void EigenVectorFromTest() const
  {
    // note: not all containers will have a value_type type member
    using value_t = typename T::value_type;
    auto evec1 = pdmath::eigen_vector_from<value_t>(HELPERS_TEST_VECTOR_VALUES);
    auto evec2 = pdmath::eigen_vector_from(values_);
    EXPECT_EQ(evec1, evec2);
  }
#endif  // !PDMATH_HAS_EIGEN3

  /**
   * `VectorFromTest` implementation.
   */
  void VectorFromTest() const
  {
    auto vec = pdmath::vector_from<T>(HELPERS_TEST_VECTOR_VALUES);
    EXPECT_EQ(vec, values_);
  }

  /**
   * `UniqueVectorFromTest` implementation.
   */
  void UniqueVectorFromTest() const
  {
    auto vec_p = pdmath::unique_vector_from<T>(HELPERS_TEST_VECTOR_VALUES);
    EXPECT_EQ(*vec_p, values_);
  }
};

using HelpersVtOneTestTypes = ::testing::Types<
  pdmath::vector_d,
  std::vector<float>,
#if PDMATH_HAS_EIGEN3
  Eigen::VectorXf,
  Eigen::Vector4f,
  Eigen::RowVectorXd,
#endif  // PDMATH_HAS_EIGEN3
  pdmath::array_d<4>
>;
TYPED_TEST_SUITE(HelpersVtOneTest, HelpersVtOneTestTypes);

/**
 * Test that `to_string()` range format overloads work as expected.
 *
 * This used to be called `PrintVectorTest` but `print_vector()` was removed.
 *
 * @todo Rename this test since it works with generic ranges. We also want to
 *  move this into a separate translation unit for `range_format.h` tests.
 */
TYPED_TEST(HelpersVtOneTest, RangeFormatTest)
{
  this->RangeFormatTest();
}

#if PDMATH_HAS_BOOST
/**
 * Test that `boost_vector_from` overloads work as expected.
 */
TYPED_TEST(HelpersVtOneTest, BoostVectorFromTest)
{
  this->BoostVectorFromTest();
}
#endif  // PDMATH_HAS_BOOST

#if PDMATH_HAS_EIGEN3
/**
 * Test that `eigen_vector_from` overloads work as expected on vectors.
 */
TYPED_TEST(HelpersVtOneTest, EigenVectorFromTest)
{
  this->EigenVectorFromTest();
}
#endif  // PDMATH_HAS_EIGEN3

/**
 * Test that `vector_from` overload using parameter pack works as expected.
 */
TYPED_TEST(HelpersVtOneTest, VectorFromTest)
{
  this->VectorFromTest();
}

/**
 * Test that `unique_vector_from` overload using parameter pack works.
 */
TYPED_TEST(HelpersVtOneTest, UniqueVectorFromTest)
{
  this->UniqueVectorFromTest();
}

#if PDMATH_HAS_EIGEN3
/**
 * Test fixture template for `eigen_vector_from` tests using Eigen matrices.
 *
 * We make this separate from `HelpersVtOneTest` since 2D `Eigen::Matrix`
 * specializations required incompatible nested initialization.
 *
 * @tparam T `Eigen::Matrix` specialization
 */
template <typename T>
class EigenVectorFromMatrixTest : public ::testing::Test {
protected:
// MSVC complains (again) about double to float truncation
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4305)
  static inline const T values_{HELPERS_TEST_MATRIX_VALUES};
PDMATH_MSVC_WARNINGS_POP()

  /**
   * Ensure that `eigen_vector_from()` works as expected.
   */
  void operator()() const
  {
    using value_t = typename T::value_type;
    auto evec1 = pdmath::eigen_vector_from<value_t>(HELPERS_TEST_VECTOR_VALUES);
    // canceling expression resolves to values_
    auto evec2 = pdmath::eigen_vector_from(values_ - values_ + values_);
    EXPECT_EQ(evec1, evec2);
  }
};

using EigenVectorFromMatrixTestTypes = ::testing::Types<
  Eigen::Matrix2f,
  Eigen::Matrix2Xd,
  Eigen::MatrixXf
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
  (*this)();
}
#endif  // PDMATH_HAS_EIGEN3

/**
 * Test fixture template for tests requiring two vector types.
 *
 * @tparam T `type_pack<T1, T2>` with `T1`, `T2` *Container* types
 */
template <typename T, typename = void>
class HelpersVtTwoTest : public ::testing::Test {};

/**
 * Partial specialization for `type_pack<T1, T2>`.
 *
 * @tparam T1 *Container* type
 * @tparam T2 *Container* type
 */
template <typename T1, typename T2>
class HelpersVtTwoTest<pdmath::testing::type_pack<T1, T2>>
  : public ::testing::Test {
protected:
  // element types must of course be the same
  static_assert(
    std::is_same_v<typename T1::value_type, typename T2::value_type>,
    "T1::value_type, T2::value_type must be the same"
  );

// MSVC complains (again) about double to float truncation
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4305)
  static inline const T1 values_1_{{HELPERS_TEST_VECTOR_VALUES}};
  static inline const T2 values_2_{{HELPERS_TEST_VECTOR_VALUES}};
PDMATH_MSVC_WARNINGS_POP()

  /**
   * `VectorFromTest` implementation.
   */
  void VectorFromTest() const
  {
    auto vec1 = pdmath::vector_from<T1>(values_2_);
    auto vec2 = pdmath::vector_from<T2>(values_1_);
    EXPECT_EQ(vec1, values_1_);
    EXPECT_EQ(vec2, values_2_);
  }

  /**
   * `UniqueVectorFromTest` implementation.
   */
  void UniqueVectorFromTest() const
  {
    auto vec1_p = pdmath::unique_vector_from<T1>(values_2_);
    auto vec2_p = pdmath::unique_vector_from<T2>(values_1_);
    EXPECT_EQ(*vec1_p, values_1_);
    EXPECT_EQ(*vec2_p, values_2_);
  }
};

using HelpersVtTwoTestTypes = ::testing::Types<
#if PDMATH_HAS_EIGEN3
  pdmath::testing::type_pack<pdmath::vector_d, Eigen::VectorXd>,
  pdmath::testing::type_pack<Eigen::Vector4f, pdmath::vector_f>,
  pdmath::testing::type_pack<Eigen::RowVectorXd, Eigen::Vector4d>,
  pdmath::testing::type_pack<pdmath::array_f<4>, Eigen::VectorXf>,
  pdmath::testing::type_pack<pdmath::array_f<4>, Eigen::RowVector4f>,
  pdmath::testing::type_pack<Eigen::RowVector4d, pdmath::vector_d>,
#endif  // PDMATH_HAS_EIGEN3
  pdmath::testing::type_pack<pdmath::vector_d, pdmath::array_d<4>>
>;
TYPED_TEST_SUITE(HelpersVtTwoTest, HelpersVtTwoTestTypes);

/**
 * Test that `vector_from` overload using a *Container* works as expected.
 */
TYPED_TEST(HelpersVtTwoTest, VectorFromTest)
{
  this->VectorFromTest();
}

/**
 * Test that `unique_vector_from` overload using a *Container* works.
 */
TYPED_TEST(HelpersVtTwoTest, UniqueVectorFromTest)
{
  this->UniqueVectorFromTest();
}

}  // namespace
