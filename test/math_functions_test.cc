/**
 * @file math_functions_test.cc
 * @author Derek Huang
 * @brief Tests for the templated math functions in `math_functions.h`
 * @copyright MIT License
 */

#include "pdmath/math_functions.h"

#include <gtest/gtest.h>

#include "pdmath/types.h"

namespace {

/**
 * Templated fixture class for math functions tests.
 */
template <typename T>
class MathFunctionsTest : public ::testing::Test {
protected:
  // points where the Himmelblau function is zero
  static constexpr pdmath::array_pair<T> hml_zero_1_ = {3, 2};
  static constexpr pdmath::array_pair<T> hml_zero_2_ = {-2.805118, 3.131312};
  static constexpr pdmath::array_pair<T> hml_zero_3_ = {-3.779310, -3.283186};
  static constexpr pdmath::array_pair<T> hml_zero_4_ = {3.584428, -1.848126};
  // point where the three-hump camel function is zero
  static constexpr pdmath::array_pair<T> thc_zero_ = {0, 0};
  // fixed tolerance for EXPECT_NEAR macros
  static constexpr T near_atol_ = 1e-8;
};

// types MathFunctionsTest will be instantiated with + register types
using MathFunctionsTestTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(MathFunctionsTest, MathFunctionsTestTypes);

/**
 * Unpack first and last element of test fixture static sequence container.
 *
 * @param name Name of a static sequence container member of test fixture
 */
#define TYPED_TEST_UNPACK_STATIC_FRONT_BACK(name) \
  TestFixture::name.front(), TestFixture::name.back()

#include <iostream>

/**
 * Test that Himmelblau function implementation zeroes properly.
 */
TYPED_TEST(MathFunctionsTest, HimmelblauZeroesTest)
{
  EXPECT_DOUBLE_EQ(
    0,
    pdmath::himmelblau(TYPED_TEST_UNPACK_STATIC_FRONT_BACK(hml_zero_1_))
  );
  // need looser bounds for the other zeroes
  EXPECT_NEAR(
    TypeParam(0),
    pdmath::himmelblau(TYPED_TEST_UNPACK_STATIC_FRONT_BACK(hml_zero_2_)),
    TestFixture::near_atol_
  );
  EXPECT_NEAR(
    TypeParam(0),
    pdmath::himmelblau(TYPED_TEST_UNPACK_STATIC_FRONT_BACK(hml_zero_3_)),
    TestFixture::near_atol_
  );
  EXPECT_NEAR(
    TypeParam(0),
    pdmath::himmelblau(TYPED_TEST_UNPACK_STATIC_FRONT_BACK(hml_zero_4_)),
    TestFixture::near_atol_
  );
}

/**
 * Test that three-hump camel function implementation zeroes properly.
 */
TYPED_TEST(MathFunctionsTest, ThreeHumpCamelZeroesTest)
{
  EXPECT_DOUBLE_EQ(
    TypeParam(0),
    pdmath::three_hump_camel(TYPED_TEST_UNPACK_STATIC_FRONT_BACK(thc_zero_))
  );
}

#undef TYPED_TEST_UNPACK_STATIC_FRONT_BACK

}  // namespace
