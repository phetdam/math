/**
 * @file type_traits_test.cc
 * @author Derek Huang
 * @brief Tests for the `type_traits` supplements in `type_traits.h`
 * @copyright MIT License
 */

#include "pdmath/type_traits.h"

#include <list>
#include <mutex>
#include <string>
#include <type_traits>

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "pdmath/types.h"
#include "pdmath/testing/utils.h"

namespace {

/**
 * Macro checking that `meta_template<T>` evalutes to the correct truth value.
 *
 * @param meta_template Template evaluating to a `bool`
 */
#define EXPECT_META_TEMPLATE_TRUTH_EQ(meta_template) \
  EXPECT_EQ(TypeParam::value, meta_template<typename TypeParam::type>)

/**
 * Test fixture template for testing `is_vector_size_constructible`.
 *
 * @tparam Tp_t `type_value_pair<T, v>` specialization
 */
template <typename Tp_t>
class TypeTraitsSizeConstructibleTest : public ::testing::Test {};

using TypeTraitsSizeConstructibleTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<pdmath::vector_d, true>,
  pdmath::testing::type_value_pair<pdmath::boost_vector_f, true>,
  pdmath::testing::type_value_pair<Eigen::VectorXd, true>,
  pdmath::testing::type_value_pair<std::list<double>, true>,
  pdmath::testing::type_value_pair<pdmath::boost_matrix_d, false>,
  pdmath::testing::type_value_pair<pdmath::array_pair_d, false>,
  pdmath::testing::type_value_pair<std::string, false>,
  pdmath::testing::type_value_pair<std::mutex, false>
>;
TYPED_TEST_SUITE(
  TypeTraitsSizeConstructibleTest, TypeTraitsSizeConstructibleTestTypes
);

/**
 * Check that the type is or is not size-constructible, as expected.
 */
TYPED_TEST(TypeTraitsSizeConstructibleTest, TruthTest)
{
  EXPECT_META_TEMPLATE_TRUTH_EQ(pdmath::is_vector_size_constructible_v);
}

}  // namespace
