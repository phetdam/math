/**
 * @file type_traits_test.cc
 * @author Derek Huang
 * @brief type_traits.h unit tests
 * @copyright MIT License
 */

#include "pdmath/type_traits.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <list>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <variant>

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "pdmath/fibonacci.h"
#include "pdmath/types.h"
#include "pdmath/testing/utils.h"
#include "pdmath/warnings.h"

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

// silence MSVC C5245 (unreferenced function with internal linkage removed)
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(5245)
#endif  // _MSC_VER
auto& operator<<(std::ostream& out, streamable value)
{
  return out << "<streamable at " << &value;
}
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER

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

/**
 * Vector wrapper class representing a range.
 *
 * @tparam T type
 */
template <typename T>
class vec_wrapper {
public:
  vec_wrapper(std::vector<T>&& values) noexcept : values_{std::move(values)} {}
  auto begin() const noexcept { return values_.begin(); }
  auto end() const noexcept { return values_.end(); }
  auto& operator[](std::size_t i) noexcept { return values_[i]; }
  auto& operator[](std::size_t i) const noexcept { return values_[i]; }
private:
  std::vector<T> values_;
};

using IsRangeTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<std::string, true>,
  pdmath::testing::type_value_pair<int, false>,
  pdmath::testing::type_value_pair<std::deque<int>, true>,
  pdmath::testing::type_value_pair<std::vector<double>, true>,
  pdmath::testing::type_value_pair<std::monostate, false>,
  pdmath::testing::type_value_pair<std::list<unsigned>, true>,
  pdmath::testing::type_value_pair<vec_wrapper<int>, true>
>;
TYPED_TEST_SUITE(IsRangeTest, IsRangeTestTypes);

// define test for is_range
PDMATH_TRAITS_TEST(IsRangeTest);

/**
 * Test fixture template for testing `is_indirectly_readable`.
 *
 * @tparam T `type_value_pair` specialization
 */
template <typename T>
class IsIndirectlyReadableTest
  : public pdmath::testing::traits_test<pdmath::is_indirectly_readable, T> {};

using IsIndirectlyReadableTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<const char*, true>,
  pdmath::testing::type_value_pair<const char&, false>,
  pdmath::testing::type_value_pair<void*, false>,
  pdmath::testing::type_value_pair<int, false>,
  pdmath::testing::type_value_pair<std::unique_ptr<std::vector<double>>, true>,
  pdmath::testing::type_value_pair<std::shared_ptr<std::string>, true>,
  pdmath::testing::type_value_pair<std::string, false>
>;
TYPED_TEST_SUITE(IsIndirectlyReadableTest, IsIndirectlyReadableTestTypes);

// define teste for is_indirectly_readable
PDMATH_TRAITS_TEST(IsIndirectlyReadableTest);

/**
 * Test fixture template for testing `is_weakly_incrementable`.
 *
 * @tparam T `type_value_pair` specialization
 */
template <typename T>
class IsWeaklyIncrementableTest
  : public pdmath::testing::traits_test<pdmath::is_weakly_incrementable, T> {};

/**
 * Type that is pre-incrementable.
 */
class pre_incrementable {
public:
  auto& operator++() noexcept { return *this; }
};

/**
 * Type that is post-incrementable.
 */
class post_incrementable {
public:
  auto operator++(int) noexcept { return *this; }
};

/**
 * Type that is both pre- and post- incrementable.
 */
class incrementable {
public:
  auto& operator++() noexcept { return *this; }
  auto operator++(int) noexcept { return *this; }
};

using IsWeaklyIncrementableTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<int, true>,
  pdmath::testing::type_value_pair<double, true>,
  pdmath::testing::type_value_pair<std::vector<int>::iterator, true>,
  pdmath::testing::type_value_pair<std::string::iterator, true>,
  pdmath::testing::type_value_pair<void, false>,
  pdmath::testing::type_value_pair<std::string, false>,
  pdmath::testing::type_value_pair<const char*, true>,
  // object type is incomplete and does not admit operator++ or operator++(int)
  pdmath::testing::type_value_pair<const void*, false>,
  // not post-incrementable
  pdmath::testing::type_value_pair<pre_incrementable, false>,
  // not pre-incrementable
  pdmath::testing::type_value_pair<post_incrementable, false>,
  // pre- and post-incrementable
  pdmath::testing::type_value_pair<incrementable, true>
>;
TYPED_TEST_SUITE(IsWeaklyIncrementableTest, IsWeaklyIncrementableTestTypes);

// define is_weakly_incrementable test
PDMATH_TRAITS_TEST(IsWeaklyIncrementableTest);

/**
 * Test fixture template for testing `is_legacy_iterator`.
 *
 * @tparam T `type_value_pair` specialization
 */
template <typename T>
class IsLegacyIteratorTest
  : public pdmath::testing::traits_test<pdmath::is_legacy_iterator, T> {};

using IsLegacyIteratorTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<std::vector<int>::iterator, true>,
  pdmath::testing::type_value_pair<std::string::iterator, true>,
  pdmath::testing::type_value_pair<const char*, true>,
  // cannot dereference with incomplete type
  pdmath::testing::type_value_pair<const void*, false>,
  pdmath::testing::type_value_pair<pdmath::basic_fibonacci_generator, true>,
  pdmath::testing::type_value_pair<pdmath::fibonacci_generator, true>
>;
TYPED_TEST_SUITE(IsLegacyIteratorTest, IsLegacyIteratorTestTypes);

// define is_legacy_iterator test
PDMATH_TRAITS_TEST(IsLegacyIteratorTest);

/**
 * Test fixture template for testing `is_equality_comparable`.
 *
 * @tparam T `type_value_pair` specialization
 */
template <typename T>
class IsEqualityComparableTest
  : public pdmath::testing::traits_test<pdmath::is_equality_comparable, T> {};

using IsEqualityComparableTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<std::vector<double>::iterator, true>,
  pdmath::testing::type_value_pair<int, true>,
  pdmath::testing::type_value_pair<const char*, true>,
  pdmath::testing::type_value_pair<const void*, true>,
  // LegacyIterator doesn't have to be equality comparable
  pdmath::testing::type_value_pair<pdmath::basic_fibonacci_generator, false>,
  // LegacyInputIterator requires equality comparable
  pdmath::testing::type_value_pair<pdmath::fibonacci_generator, true>,
  pdmath::testing::type_value_pair<std::mutex, false>
>;
TYPED_TEST_SUITE(IsEqualityComparableTest, IsEqualityComparableTestTypes);

// define is_equality_comparable test
PDMATH_TRAITS_TEST(IsEqualityComparableTest);

/**
 * Test fixture template for testing `is_inequality_comparable`.
 *
 * @tparam T `type_value_pair` specialization
 */
template <typename T>
class IsInequalityComparableTest
  : public pdmath::testing::traits_test<pdmath::is_inequality_comparable, T> {};

// reusing types from IsEqualityComparableTest
using IsInequalityComparableTestTypes = IsEqualityComparableTestTypes;
TYPED_TEST_SUITE(IsInequalityComparableTest, IsInequalityComparableTestTypes);

// define is_inequality_comparable test
PDMATH_TRAITS_TEST(IsInequalityComparableTest);

/**
 * Test fixture template for testing `iter_value`.
 *
 * @tparam T `type_pair_wrapper` specialization
 */
template <typename T>
class IterValueTest
  : public pdmath::testing::traits_test<pdmath::iter_value, T> {};

using IterValueTestTypes = ::testing::Types<
  pdmath::testing::type_pair_wrapper<std::vector<int>::iterator, int>,
  pdmath::testing::type_pair_wrapper<const double*, double>,
  pdmath::testing::type_pair_wrapper<
    pdmath::basic_fibonacci_generator, std::uint_fast64_t
  >,
  pdmath::testing::type_pair_wrapper<
    pdmath::fibonacci_generator, std::uint_fast64_t
  >,
  // test against expected incorrect types too
  pdmath::testing::type_pair_wrapper<const double*, pdmath::always_false<int>>,
  pdmath::testing::type_pair_wrapper<
    std::deque<unsigned>::iterator, pdmath::always_false<double>
  >
>;
TYPED_TEST_SUITE(IterValueTest, IterValueTestTypes);

// define iter_value test
PDMATH_TRAITS_TEST(IterValueTest);

/**
 * Test fixture template for testing `range_value`.
 *
 * @tparam T `type_pair_wrapper` specialization
 */
template <typename T>
class RangeValueTest
  : public pdmath::testing::traits_test<pdmath::range_value, T> {};

using RangeValueTestTypes = ::testing::Types<
  pdmath::testing::type_pair_wrapper<std::vector<double>, double>,
  pdmath::testing::type_pair_wrapper<vec_wrapper<int>, int>,
  pdmath::testing::type_pair_wrapper<std::deque<unsigned>, unsigned>,
  pdmath::testing::type_pair_wrapper<std::string, char>,
  // test against expected incorrect types too
  pdmath::testing::type_pair_wrapper<
    std::vector<int>, pdmath::always_false<std::string>
  >,
  pdmath::testing::type_pair_wrapper<std::string, pdmath::always_false<int>>
>;
TYPED_TEST_SUITE(RangeValueTest, RangeValueTestTypes);

// define range_value test
PDMATH_TRAITS_TEST(RangeValueTest);

/**
 * Test fixture template for testing `is_legacy_input_iterator`.
 *
 * @tparam T `type_value_pair` specialization
 */
template <typename T>
class IsLegacyInputIteratorTest
  : public pdmath::testing::traits_test<pdmath::is_legacy_input_iterator, T> {};

using IsLegacyInputIteratorTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<std::vector<int>::iterator, true>,
  pdmath::testing::type_value_pair<std::deque<unsigned>::iterator, true>,
  pdmath::testing::type_value_pair<std::vector<int>::const_iterator, true>,
  // primitive types don't have members but they qualify as input iterators
  pdmath::testing::type_value_pair<const char*, false>,
  pdmath::testing::type_value_pair<const void*, false>,
  pdmath::testing::type_value_pair<int, false>,
  pdmath::testing::type_value_pair<std::string::iterator, true>,
  pdmath::testing::type_value_pair<std::string, false>,
  pdmath::testing::type_value_pair<pdmath::basic_fibonacci_generator, false>,
  pdmath::testing::type_value_pair<pdmath::fibonacci_generator, true>
>;
TYPED_TEST_SUITE(IsLegacyInputIteratorTest, IsLegacyInputIteratorTestTypes);

// define is_legacy_input_iterator test
PDMATH_TRAITS_TEST(IsLegacyInputIteratorTest);

}  // namespace
