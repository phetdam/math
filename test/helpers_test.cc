/**
 * @file helpers_test.cc
 * @author Derek Huang
 * @brief Tests for helpers defined in helpers.h
 * @copyright MIT License
 */

#include "pdmath/helpers.h"

#include <sstream>
#include <vector>

#include <Eigen/Core>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/types.h"

/**
 * Values used in the HelpersTest test fixture for populating `std::vector`.
 *
 * Used later to test the variadic template versions of `boost_vector_from`
 * and `eigen_vector_from` to ensure that they work as intended.
 */
#define HELPERS_TEST_VECTOR_VALUES 1., 34, 4.5, 9.82

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
  auto header = pdmath::print_example_header(ex_path_, false);
  EXPECT_EQ(header, pdmath::print_example_header(ex_path_.c_str(), false));
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
  std::stringstream ss_def;
  for (const auto& v : values_) {
    ss_def << v;
    if (v != values_.back()) {
      ss_def << " ";
    }
  }
  // formatted string printed using custom vector_print_policy
  pdmath::vector_print_policy print_policy(",", 2, "[", 2, "]", 2);
  std::stringstream ss_cust;
  ss_cust << "[  ";
  for (const auto& v : values_) {
    ss_cust << v;
    if (v != values_.back()) {
      ss_cust << ",";
    }
    ss_cust << "  ";
  }
  ss_cust << "]";
  EXPECT_EQ(ss_def.str(), pdmath::print_vector(values_, false));
  EXPECT_EQ(ss_cust.str(), pdmath::print_vector(values_, print_policy, false));
}

/**
 * Test that `boost_vector_from` overloads work as expected.
 */
TEST_F(HelpersTest, BoostVectorFromTest)
{
  auto bvec1 = pdmath::boost_vector_from(values_);
  auto bvec2 = pdmath::boost_vector_from<double>(HELPERS_TEST_VECTOR_VALUES);
  EXPECT_EQ(bvec1.size(), bvec2.size());
  // boost::numeric::ublas::vector does not define operator==, so use gmock
  EXPECT_THAT(bvec1, ::testing::ElementsAreArray(bvec2));
}

/**
 * Test that `eigen_vector_from` overloads work as expected.
 */
TEST_F(HelpersTest, EigenVectorFromTest)
{
  auto evec1 = pdmath::eigen_vector_from(values_);
  auto evec2 = pdmath::eigen_vector_from<double>(HELPERS_TEST_VECTOR_VALUES);
  EXPECT_EQ(evec1, evec2);
  // eigen_vector_from should work on a matrix
  Eigen::Matrix2f m2;
  m2 << HELPERS_TEST_VECTOR_VALUES;
  // need to cast double to float since m2 has float scalar type. of course we
  // cannot do the reverse since float is lower precision than double
  EXPECT_EQ(evec1.cast<float>(), pdmath::eigen_vector_from(m2));
  // should also work on a row vector or a fixed-size vector
  Eigen::RowVector4d rv{{HELPERS_TEST_VECTOR_VALUES}};
  EXPECT_EQ(evec1, pdmath::eigen_vector_from(rv));
  Eigen::VectorXd v{{HELPERS_TEST_VECTOR_VALUES}};
  EXPECT_EQ(evec1, pdmath::eigen_vector_from(v));
}

}  // namespace
