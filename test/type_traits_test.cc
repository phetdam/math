/**
 * @file type_traits_test.cc
 * @author Derek Huang
 * @brief Tests for the `type_traits` supplements in `type_traits.h`
 * @copyright MIT License
 */

#include "pdmath/type_traits.h"

#include <cstddef>
#include <deque>
#include <list>
#include <mutex>
#include <string>
#include <type_traits>
#include <typeinfo>

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
 * Simple user-defined type constructible via a `size_t`.
 */
class size_constructible_1 {
public:
  constexpr size_constructible_1(std::size_t) noexcept {}
};

/**
 * Simple user-defined type constructible via an `int`.
 *
 * This is used to test the fact that `is_constructible` uses
 * `T(std::declval<Ts>()...)` instead of `T{std::declval<Ts>()...}` as the
 * latter would not accept a narrowing conversion.
 */
class size_constructible_2 {
public:
  constexpr size_constructible_2(int) noexcept {}
};

/**
 * Test fixture template for testing `is_size_constructible`.
 *
 * @tparam T `type_value_pair<T, v>` specialization
 */
template <typename T>
class TypeTraitsSizeConstructibleTest : public ::testing::Test {};

using TypeTraitsSizeConstructibleTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<pdmath::vector_d, true>,
  pdmath::testing::type_value_pair<pdmath::boost_vector_f, true>,
  pdmath::testing::type_value_pair<Eigen::VectorXd, true>,
  pdmath::testing::type_value_pair<std::list<double>, true>,
  pdmath::testing::type_value_pair<pdmath::boost_matrix_d, false>,
  pdmath::testing::type_value_pair<pdmath::array_pair_d, false>,
  pdmath::testing::type_value_pair<std::string, false>,
  pdmath::testing::type_value_pair<std::mutex, false>,
  pdmath::testing::type_value_pair<std::deque<double>, true>,
  pdmath::testing::type_value_pair<size_constructible_1, true>,
  pdmath::testing::type_value_pair<size_constructible_2, true>
>;
TYPED_TEST_SUITE(
  TypeTraitsSizeConstructibleTest, TypeTraitsSizeConstructibleTestTypes
);

/**
 * Check that the type is or is not size-constructible, as expected.
 */
TYPED_TEST(TypeTraitsSizeConstructibleTest, TruthTest)
{
  EXPECT_META_TEMPLATE_TRUTH_EQ(pdmath::is_size_constructible_v);
}

/**
 * Template class castable to a `std::string` giving its mangled `typeid` name.
 *
 * @tparam T type
 */
template <typename T>
class printable {
public:
  operator std::string() const { return typeid(T).name(); }
};

/**
 * Class [implicitly] castable to a `std::string` containing `"hello"`.
 */
class implicit_hello {
public:
  operator std::string() const { return "hello"; }
};

/**
 * Class castable to a `std::string` containing `"hello"`, but only explicitly.
 */
class explicit_hello {
public:
  explicit operator std::string() const { return "hello"; }
};

/**
 * User-defined type that has no conversion to `std::string`.
 */
struct not_string_convertible {};

/**
 * Test fixture template for testing `has_operator_string`.
 *
 * @tparam Tp_t `type_value_pair<T, v>` specialization
 */
template <typename Tp_t>
class TypeTraitsHasOperatorStringTest : public ::testing::Test {};

using TypeTraitsHasOperatorStringTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<printable<pdmath::array_pair_f>, true>,
  pdmath::testing::type_value_pair<implicit_hello, true>,
  pdmath::testing::type_value_pair<explicit_hello, true>,
  pdmath::testing::type_value_pair<pdmath::boost_vector_d, false>,
  pdmath::testing::type_value_pair<std::list<int>, false>,
  pdmath::testing::type_value_pair<std::mutex, false>,
  pdmath::testing::type_value_pair<not_string_convertible, false>
>;
TYPED_TEST_SUITE(
  TypeTraitsHasOperatorStringTest, TypeTraitsHasOperatorStringTestTypes
);

/**
 * Check that `has_operator_string` is substituting correctly.
 */
TYPED_TEST(TypeTraitsHasOperatorStringTest, TruthTest)
{
  EXPECT_META_TEMPLATE_TRUTH_EQ(pdmath::has_operator_string_v);
}

}  // namespace
