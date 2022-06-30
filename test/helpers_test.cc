/**
 * @file helpers_test.cc
 * @author Derek Huang
 * @brief Tests for helpers defined in helpers.h
 * @copyright MIT License
 */

#include "pdmath/helpers.h"

#include <sstream>

#include <gtest/gtest.h>

#include "pdmath/types.h"

/**
 * Values used in the HelpersTest test fixture for populating `std::vector`.
 *
 * Used later to test the variadic template version of `boost_vector_from`.
 */
#define HELPERS_TEST_VECTOR_VALUES 1., 34, 4.5, 9.82

namespace pdmath {
namespace tests {

namespace {

/**
 * Test fixture class for helper tests.
 */
class HelpersTest : public ::testing::Test {
protected:
  // name of exercise example path we want to show
  static const std::string ex_name_;
  // actual full path of exercise example path
  static const std::string ex_path_;
  // frame to display around ex_name_, i.e. a bunch of dashes same length
  static const std::string ex_frame_;
  static const std::vector<double> values_;
};

const std::string
HelpersTest::ex_name_ = "dir1/exampleX.cc";

const std::string
HelpersTest::ex_path_ = "math/examples/" + HelpersTest::ex_name_;

const std::string
HelpersTest::ex_frame_ = std::string(HelpersTest::ex_name_.size(), '-');

const std::vector<double> HelpersTest::values_ = {HELPERS_TEST_VECTOR_VALUES};

/**
 * Test that `print_example_header` overloads work as expected.
 */
TEST_F(HelpersTest, PrintExampleHeaderTest)
{
  auto header = print_example_header(ex_path_, false);
  EXPECT_EQ(header, print_example_header(ex_path_.c_str(), false));
  ASSERT_EQ(
    "+" + ex_frame_ + "+\n|" + ex_name_ + "|\n+" + ex_frame_ + "+", header
  );
}

/**
 * Test that `print_vector` overloads work as expected.
 */
TEST_F(HelpersTest, PrintVectorTest)
{
  // formatted string printed by default
  std::stringstream ss_default;
  for (const auto& v : values_) {
    ss_default << v;
    if (v != values_.back()) {
      ss_default << " ";
    }
  }
  // formatted string printed using custom vector_print_policy
  vector_print_policy print_policy(",", 2, "[", 2, "]", 2);
  std::stringstream ss_special;
  ss_special << "[  ";
  for (const auto& v : values_) {
    ss_special << v;
    if (v != values_.back()) {
      ss_special << ",";
    }
    ss_special << "  ";
  }
  ss_special << "]";
  EXPECT_EQ(ss_default.str(), print_vector(values_, false));
  EXPECT_EQ(ss_special.str(), print_vector(values_, print_policy, false));
}

/**
 * Test that `boost_vector_from` overloads work as expected.
 */
TEST_F(HelpersTest, BoostVectorFromTest)
{
  auto bvec1 = boost_vector_from(values_);
  auto bvec2 = boost_vector_from<double>(HELPERS_TEST_VECTOR_VALUES);
  // use custom operator== definition for boost_vector<T> in ASSERT_TRUE since
  // using ASSERT_EQ does not find overload using ADL and doesn't compile.
  // overloaded operator== is defined in pdmath/types.h in namespace pdmath.
  EXPECT_TRUE(bvec1 == bvec2);
}

}  // namespace

}  // namespace pdmath::tests
}  // namespace pdmath
