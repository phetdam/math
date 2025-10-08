/**
 * @file type_traits.h
 * @author Derek Huang
 * @brief C++ header for additional type traits
 * @copyright MIT License
 */

#ifndef PDMATH_TYPE_TRAITS_H_
#define PDMATH_TYPE_TRAITS_H_

#include <cstddef>
// TODO: consider using <iosfwd> and std::basic_ostream<char> instead
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

namespace pdmath {

/**
 * Check is a type is constructible from a `size_t`.
 *
 * Examples would be `std::vector`, which can be constructed with a given
 * number of value-initialized elements, or an Eigen row/column vector.
 *
 * @tparam T type
 */
template <typename T>
struct is_size_constructible : std::is_constructible<T, std::size_t> {};

/**
 * Indicate if a type is constructible from a `size_t`.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_size_constructible_v = is_size_constructible<T>::value;

/**
 * Indicate if a type is constructible from a `size_t`.
 *
 * @deprecated This is provided for compatibility with existing code.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_vector_size_constructible_v = is_size_constructible_v<T>;

/**
 * Checks if a type is castable to `std::string`.
 *
 * That is, the type `T` implements or inherits `operator std::string`, which
 * may or may not be declared as explicit, i.e. cast required.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct has_operator_string : std::false_type {};

template <typename T>
struct has_operator_string<
  T, std::void_t<decltype(std::declval<T>().operator std::string())>>
  : std::true_type {};

/**
 * Helper for `has_operator_string` to get the truth value.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool has_operator_string_v = has_operator_string<T>::value;

/**
 * Constraint traits type.
 *
 * This is inspired by something similar in Boost.Asio that enables more
 * unified implementation of pre-C++20 template constraints instead of using
 * SFINAE with template type parameters. This avoids the following:
 *
 *  - `std::enable_if_t` being verbose with return types + not supporting `auto`
 *  - `std::enable_if_t` being used to constrain function templates via default
 *    template type parameter. The compiler will see this as a redeclaration of
 *    the same function template. See the cppreference documentation for
 *    details: https://en.cppreference.com/w/cpp/types/enable_if.html
 *
 * See `constraint_t` for an example of how to use the constraint traits.
 *
 * @tparam V `constexpr` truth value
 */
template <bool V>
struct constraint {};

/**
 * SFINAE helper for using the constraint traits with function templates.
 *
 * This is more convenient that `std::enable_if_t` since it avoids the pitfalls
 * detailed in the `constraint` documentation. It can be used the same way as
 * its Boost.Asio analogue; for example:
 *
 * @code{.cc}
 * template <typename T>
 * auto float_only(T v, constraint_t<std::is_floating_point_v<T>> = 0)
 * {
 *   return v + 1;
 * }
 * @endcode
 *
 * In cases where an optional argument is not allowed or not desirable, one can
 * use it like `std::enable_if_t` as a non-type template parameter:
 *
 * @code{.cc}
 * // constrain to only floating-point value types while allowing additional
 * // constrained function templates, e.g. for integral value types
 * template <typename T, typename A, constraint_t<std::is_floating_point_v<T>> = 0>
 * auto& operator<<(std::ostream& out, const std::vector<T, A>& vec)
 * {
 *   // ...
 * }
 * @endcode
 *
 * @tparam V `constexpr` truth value
 */
template <bool V>
using constraint_t = typename constraint<V>::type;

/**
 * True specialization that provides the `int` type alias.
 */
template <>
struct constraint<true> {
  using type = int;
};

/**
 * Traits to indicate if a type can be streamed to `std::ostream`.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_ostreamable : std::false_type {};

/**
 * True specialization for a type that is streamable to `std::ostream`.
 *
 * @tparam T type
 */
template <typename T>
struct is_ostreamable<
  T, std::void_t<decltype(std::declval<std::ostream>() << std::declval<T>())> >
  : std::true_type {};

/**
 * Indicate if a type is streamable to `std::ostream`.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_ostreamable_v = is_ostreamable<T>::value;

/**
 * Traits type to indicate a pre-C++20 range.
 *
 * This is a very weak specification of a range where the type needs to only
 * provide start and end iterators via `std::begin` and `std::end`.
 *
 * @todo Strengthen so we actually check that iterators are returned.
 *
 * @tparam T type
 */
template <typename T, typename = void, typename = void>
struct is_range : std::false_type {};

/**
 * True specialization for a type that provides begin + end iterators.
 *
 * @tparam T type
 */
template <typename T>
struct is_range<
  T,
  std::void_t<decltype(std::begin(std::declval<T>()))>,
  std::void_t<decltype(std::end(std::declval<T>()))>
> : std::true_type {};

/**
 * Indicate that a type is a range-like type.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_range_v = is_range<T>::value;

}  // namespace pdmath

#endif  // PDMATH_TYPE_TRAITS_H_
