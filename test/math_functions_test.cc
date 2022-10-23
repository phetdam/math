/**
 * @file math_functions_test.cc
 * @author Derek Huang
 * @brief Tests for the templated math functions in `math_functions.h`
 * @copyright MIT License
 */

#include "pdmath/math_functions.h"

#include <gtest/gtest.h>

#include "pdmath/types.h"
#include "pdmath/warnings.h"
#include "pdmath/testing/macros.h"

namespace {

/**
 * Templated fixture class for math functions tests.
 */
template <typename T>
class MathFunctionsTest : public ::testing::Test {
protected:
  // points where the Himmelblau function is zero
  static constexpr pdmath::array_pair<T> hml_zero_1_ = {HIMMELBLAU_ZERO_1};
// disable truncation warning when initializing from double
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
  static constexpr pdmath::array_pair<T> hml_zero_2_ = {HIMMELBLAU_ZERO_2};
  static constexpr pdmath::array_pair<T> hml_zero_3_ = {HIMMELBLAU_ZERO_3};
  static constexpr pdmath::array_pair<T> hml_zero_4_ = {HIMMELBLAU_ZERO_4};
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
  // point where the three-hump camel function is zero
  static constexpr pdmath::array_pair<T> thc_zero_ = {0, 0};
  // fixed tolerance for EXPECT_NEAR macros
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4305)
#endif  // _MSC_VER
  static constexpr T near_atol_ = 1e-10;
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
};

// types MathFunctionsTest will be instantiated with + register types
using MathFunctionsTestTypes = ::testing::Types<float, double>;
TYPED_TEST_SUITE(MathFunctionsTest, MathFunctionsTestTypes);

/**
 * Unpack front, back of templated `::testing::Test` static sequence container.
 *
 * @param name Static sequence container member of templated `::testing::Test`
 */
#define TYPED_TEST_UNPACK_STATIC_FRONT_BACK(name) \
  TestFixture::name.front(), TestFixture::name.back()

/**
 * Check that `func` is zero at a static member of `MathFunctionsTest`.
 *
 * Uses Google Test `EXPECT_DOUBLE_EQ` macro to do the comparison.
 *
 * @param func Callable taking `const T&`, `const T&`, returning `T`
 * @param zero `const pdmath::array_pair<T>&` `MathFunctionsTest` static member
 */
#define MathFunctionsTest_EXPECT_STATIC_ZERO(func, zero) \
  EXPECT_DOUBLE_EQ(0, func(TYPED_TEST_UNPACK_STATIC_FRONT_BACK(zero)))

/**
 * Check that `func` is near zero at a static member of `MathFunctionsTest`.
 *
 * Uses Google Test `EXPECT_NEAR` macro and `MathFunctionsTest<T>::near_atol_`.
 *
 * @param func Callable taking `const T&`, `const T&`, returning `T`
 * @param zero `const pdmath::array_pair<T>&` `MathFunctionsTest` static member
 */
#define MathFunctionsTest_EXPECT_STATIC_ZERO_NEAR(func, zero) \
  EXPECT_NEAR( \
    0, \
    func(TYPED_TEST_UNPACK_STATIC_FRONT_BACK(zero)), \
    TestFixture::near_atol_ \
  )

/**
 * Test that Himmelblau function implementation zeroes properly.
 */
TYPED_TEST(MathFunctionsTest, HimmelblauZerosTest)
{
  MathFunctionsTest_EXPECT_STATIC_ZERO(pdmath::himmelblau, hml_zero_1_);
  // need looser bounds for the other zeroes
  MathFunctionsTest_EXPECT_STATIC_ZERO_NEAR(pdmath::himmelblau, hml_zero_1_);
  MathFunctionsTest_EXPECT_STATIC_ZERO_NEAR(pdmath::himmelblau, hml_zero_2_);
  MathFunctionsTest_EXPECT_STATIC_ZERO_NEAR(pdmath::himmelblau, hml_zero_3_);
}

/**
 * Test that three-hump camel function implementation zeroes properly.
 */
TYPED_TEST(MathFunctionsTest, ThreeHumpCamelZerosTest)
{
  MathFunctionsTest_EXPECT_STATIC_ZERO(pdmath::three_hump_camel, thc_zero_);
}

}  // namespace
