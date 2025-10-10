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
#include <ostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <variant>

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "pdmath/types.h"
#include "pdmath/testing/utils.h"

namespace {

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
 * @tparam T `type_value_pair<U, v>` specialization
 */
template <typename T>
class SizeConstructibleTest
  : public pdmath::testing::traits_test<pdmath::is_size_constructible, T> {};

using SizeConstructibleTestTypes = ::testing::Types<
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
TYPED_TEST_SUITE(SizeConstructibleTest, SizeConstructibleTestTypes);

// define test for is_size_constructible
PDMATH_TRAITS_TEST(SizeConstructibleTest);

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
 * @tparam T `type_value_pair<U, v>` specialization
 */
template <typename T>
class HasOperatorStringTest
  : public pdmath::testing::traits_test<pdmath::has_operator_string, T> {};

using HasOperatorStringTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<printable<pdmath::array_pair_f>, true>,
  pdmath::testing::type_value_pair<implicit_hello, true>,
  pdmath::testing::type_value_pair<explicit_hello, true>,
  pdmath::testing::type_value_pair<pdmath::boost_vector_d, false>,
  pdmath::testing::type_value_pair<std::list<int>, false>,
  pdmath::testing::type_value_pair<std::mutex, false>,
  pdmath::testing::type_value_pair<not_string_convertible, false>
>;
TYPED_TEST_SUITE(HasOperatorStringTest, HasOperatorStringTestTypes);

// define test for has_operator_string
PDMATH_TRAITS_TEST(HasOperatorStringTest);

/**
 * Test fixture template for testing `is_ostreamable`.
 *
 * @tparam T `type_value_pair<U, v>` specialization
 */
template <typename T>
class IsOstreamableTest
  : public pdmath::testing::traits_test<pdmath::is_ostreamable, T> {};

/**
 * User-defined type with `operator<<`.
 */
struct streamable {};

auto& operator<<(std::ostream& out, streamable value)
{
  return out << "<streamable at " << &value;
}

/**
 * User-defined type with no `operator<<`.
 */
struct not_streamable {};

using IsOstreamableTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<std::string, true>,
  pdmath::testing::type_value_pair<int, true>,
  pdmath::testing::type_value_pair<void*, true>,
  pdmath::testing::type_value_pair<streamable, true>,
  pdmath::testing::type_value_pair<not_streamable, false>,
  pdmath::testing::type_value_pair<double, true>
>;
TYPED_TEST_SUITE(IsOstreamableTest, IsOstreamableTestTypes);

// define test for is_ostreamable
PDMATH_TRAITS_TEST(IsOstreamableTest);

/**
 * Test fixture template for testing `is_range`.
 *
 * @tparam T `type_value_pair` specialization
 */
template <typename T>
class IsRangeTest : public pdmath::testing::traits_test<pdmath::is_range, T> {};

// TODO: add user-defined range-like type

using IsRangeTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<std::string, true>,
  pdmath::testing::type_value_pair<int, false>,
  pdmath::testing::type_value_pair<std::deque<int>, true>,
  pdmath::testing::type_value_pair<std::vector<double>, true>,
  pdmath::testing::type_value_pair<std::monostate, false>,
  pdmath::testing::type_value_pair<std::list<unsigned>, true>
>;
TYPED_TEST_SUITE(IsRangeTest, IsRangeTestTypes);

// define test for is_range
PDMATH_TRAITS_TEST(IsRangeTest);

}  // namespace
