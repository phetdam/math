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
#include <vector>

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "pdmath/common.h"
#include "pdmath/features.h"
#include "pdmath/fibonacci.h"
#include "pdmath/testing/utils.h"
#include "pdmath/types.h"
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

// note: test types need to be defined via types alias as conditional
// compilation directives cannot not be used in the middle of macros
using SizeConstructibleTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<pdmath::vector_d, true>,
#if PDMATH_HAS_BOOST
  pdmath::testing::type_value_pair<pdmath::boost_vector_f, true>,
#endif  // PDMATH_HAS_BOOST
  pdmath::testing::type_value_pair<Eigen::VectorXd, true>,
  pdmath::testing::type_value_pair<std::list<double>, true>,
#if PDMATH_HAS_BOOST
  pdmath::testing::type_value_pair<pdmath::boost_matrix_d, false>,
#endif  // PDMATH_HAS_BOOST
  pdmath::testing::type_value_pair<pdmath::array_pair_d, false>,
  pdmath::testing::type_value_pair<std::string, false>,
  pdmath::testing::type_value_pair<std::mutex, false>,
  pdmath::testing::type_value_pair<std::deque<double>, true>,
  pdmath::testing::type_value_pair<size_constructible_1, true>,
  pdmath::testing::type_value_pair<size_constructible_2, true>
>;
// define is_size_constructible tests
PDMATH_TYPE_TRAITS_TEST(
  SizeConstructibleTest,
  pdmath::is_size_constructible,
  SizeConstructibleTestTypes
);

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

// note: test types need to be defined via types alias as conditional
// compilation directives cannot not be used in the middle of macros
using HasOperatorStringTestTypes = ::testing::Types<
  pdmath::testing::type_value_pair<printable<pdmath::array_pair_f>, true>,
  pdmath::testing::type_value_pair<implicit_hello, true>,
  pdmath::testing::type_value_pair<explicit_hello, true>,
#if PDMATH_HAS_BOOST
  pdmath::testing::type_value_pair<pdmath::boost_vector_d, false>,
#endif  // PDMATH_HAS_BOOST
  pdmath::testing::type_value_pair<std::list<int>, false>,
  pdmath::testing::type_value_pair<std::mutex, false>,
  pdmath::testing::type_value_pair<not_string_convertible, false>
>;
// define has_operator_string tests
PDMATH_TYPE_TRAITS_TEST(
  HasOperatorStringTest,
  pdmath::has_operator_string,
  HasOperatorStringTestTypes
);

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

// define is_ostreamable tsets
PDMATH_TYPE_TRAITS_TEST(
  IsOstreamableTest,
  pdmath::is_ostreamable,
  PDMATH_IDENTITY(
    ::testing::Types<
      pdmath::testing::type_value_pair<std::string, true>,
      pdmath::testing::type_value_pair<int, true>,
      pdmath::testing::type_value_pair<void*, true>,
      pdmath::testing::type_value_pair<streamable, true>,
      pdmath::testing::type_value_pair<not_streamable, false>,
      pdmath::testing::type_value_pair<double, true>
    >
  )
);

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

// define is_range tests
PDMATH_TYPE_TRAITS_TEST(
  IsRangeTest,
  pdmath::is_range,
  PDMATH_IDENTITY(
    ::testing::Types<
      pdmath::testing::type_value_pair<std::string, true>,
      pdmath::testing::type_value_pair<int, false>,
      pdmath::testing::type_value_pair<std::deque<int>, true>,
      pdmath::testing::type_value_pair<std::vector<double>, true>,
      pdmath::testing::type_value_pair<std::monostate, false>,
      pdmath::testing::type_value_pair<std::list<unsigned>, true>,
      pdmath::testing::type_value_pair<vec_wrapper<int>, true>
    >
  )
);

// define is_indirectly_readable tests
PDMATH_TYPE_TRAITS_TEST(
  IsIndirectlyReadableTest,
  pdmath::is_indirectly_readable,
  PDMATH_IDENTITY(
    ::testing::Types<
      pdmath::testing::type_value_pair<const char*, true>,
      pdmath::testing::type_value_pair<const char&, false>,
      pdmath::testing::type_value_pair<void*, false>,
      pdmath::testing::type_value_pair<int, false>,
      pdmath::testing::type_value_pair<std::unique_ptr<std::vector<double>>, true>,
      pdmath::testing::type_value_pair<std::shared_ptr<std::string>, true>,
      pdmath::testing::type_value_pair<std::string, false>,
      pdmath::testing::type_value_pair<std::vector<int>::iterator, true>,
      pdmath::testing::type_value_pair<std::string::iterator, true>
    >
  )
);

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

// define is_weakly_incrementable tests
PDMATH_TYPE_TRAITS_TEST(
  IsWeaklyIncrementableTestTypes,
  pdmath::is_weakly_incrementable,
  PDMATH_IDENTITY(
    ::testing::Types<
      pdmath::testing::type_value_pair<int, true>,
      pdmath::testing::type_value_pair<double, true>,
      pdmath::testing::type_value_pair<std::vector<int>::iterator, true>,
      pdmath::testing::type_value_pair<std::string::iterator, true>,
      pdmath::testing::type_value_pair<void, false>,
      pdmath::testing::type_value_pair<std::string, false>,
      pdmath::testing::type_value_pair<const char*, true>,
      // object type incomplete + does not admit operator++ or operator++(int)
      pdmath::testing::type_value_pair<const void*, false>,
      // not post-incrementable
      pdmath::testing::type_value_pair<pre_incrementable, false>,
      // not pre-incrementable
      pdmath::testing::type_value_pair<post_incrementable, false>,
      // pre- and post-incrementable
      pdmath::testing::type_value_pair<incrementable, true>
    >
  )
);

// define is_legacy_iterator tests
PDMATH_TYPE_TRAITS_TEST(
  IsLegacyIteratorTest,
  pdmath::is_legacy_iterator,
  PDMATH_IDENTITY(
    ::testing::Types<
      pdmath::testing::type_value_pair<std::vector<int>::iterator, true>,
      pdmath::testing::type_value_pair<std::string::iterator, true>,
      pdmath::testing::type_value_pair<const char*, true>,
      // cannot dereference with incomplete type
      pdmath::testing::type_value_pair<const void*, false>,
      pdmath::testing::type_value_pair<pdmath::basic_fibonacci_generator, true>,
      pdmath::testing::type_value_pair<pdmath::fibonacci_generator, true>
    >
  )
);

// note: reused by IsInequalityComparableTest
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
// define is_equality_comparable tests
PDMATH_TYPE_TRAITS_TEST(
  IsEqualityComparableTest,
  pdmath::is_equality_comparable,
  IsEqualityComparableTestTypes
);

/**
 * Helper traits to apply `is_equality_comparable_with<T, U>` to a type pair.
 *
 * @tparam T type
 */
template <typename T>
struct is_equality_comparable_with {};

/**
 * Partial specialization for the `type_pair_wrapper<T, U>`.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U>
struct is_equality_comparable_with<pdmath::testing::type_pair_wrapper<T, U>>
  : pdmath::is_equality_comparable_with<T, U> {};

/**
 * Template type alias for a `type_value_pair<type_pair_wrapper<T, U>, bool>`.
 *
 * @tparam T First type
 * @tparam U Second type
 * @tparam B Boolean value
 */
template <typename T, typename U, bool B>
using type_pair_case = pdmath::testing::type_value_pair<
  pdmath::testing::type_pair_wrapper<T, U>, B
>;

// define is_equality_comparable_with tests
PDMATH_TYPE_TRAITS_TEST(
  IsEqualityComparableWithTest,
  is_equality_comparable_with,
  PDMATH_IDENTITY(
    ::testing::Types<
      type_pair_case<int, double, true>,
      type_pair_case<int, std::string, false>,
      type_pair_case<std::mutex, int, false>,
      type_pair_case<std::string, const std::string&, true>,
      type_pair_case<float, double, true>,
      type_pair_case<void*, void*, true>,
      type_pair_case<char, std::variant<int, double>, false>
    >
  )
);

// define is_inequality_comparable tests
PDMATH_TYPE_TRAITS_TEST(
  IsInequalityComparableTest,
  pdmath::is_inequality_comparable,
  // note: also used by IsEqualityComparableTest
  IsEqualityComparableTestTypes
);

// define iter_value tests
PDMATH_TYPE_TRAITS_TEST(
  IterValueTest,
  pdmath::iter_value,
  PDMATH_IDENTITY(
    ::testing::Types<
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
    >
  )
);

// define range_value tests
PDMATH_TYPE_TRAITS_TEST(
  RangeValueTest,
  pdmath::range_value,
  PDMATH_IDENTITY(
    ::testing::Types<
      pdmath::testing::type_pair_wrapper<std::vector<double>, double>,
      pdmath::testing::type_pair_wrapper<vec_wrapper<int>, int>,
      pdmath::testing::type_pair_wrapper<std::deque<unsigned>, unsigned>,
      pdmath::testing::type_pair_wrapper<std::string, char>,
      // test against expected incorrect types too
      pdmath::testing::type_pair_wrapper<
        std::vector<int>, pdmath::always_false<std::string>
      >,
      pdmath::testing::type_pair_wrapper<std::string, pdmath::always_false<int>>
    >
  )
);

// define is_legacy_input_iterator tests
PDMATH_TYPE_TRAITS_TEST(
  IsLegacyInputIteratorTest,
  pdmath::is_legacy_input_iterator,
  PDMATH_IDENTITY(
    ::testing::Types<
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
    >
  )
);

}  // namespace
