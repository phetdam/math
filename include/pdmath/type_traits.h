/**
 * @file type_traits.h
 * @author Derek Huang
 * @brief Extensions to the C++ standard library `<type_traits>` header.
 * @copyright MIT License
 */

#ifndef PDMATH_TYPE_TRAITS_H_
#define PDMATH_TYPE_TRAITS_H_

#include <string>
#include <type_traits>
#include <utility>

namespace pdmath {

/**
 * Checks if a vector type is "size-constructible".
 *
 * That is, the type is a *Container* with a constructor that sets its capacity
 * to a specific size, where the size type is `size_type` or `Eigen::Index`.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_vector_size_constructible : std::false_type {};

template <typename T>
struct is_vector_size_constructible<T, std::void_t<typename T::size_type>>
  : std::is_constructible<T, typename T::size_type> {};

template <typename T>
struct is_vector_size_constructible<T, std::void_t<typename T::Index>>
  : std::is_constructible<T, typename T::Index> {};

/**
 * Helper for `is_vector_size_constructible` to get the truth value.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool
is_vector_size_constructible_v = is_vector_size_constructible<T>::value;

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

}  // namespace pdmath

#endif  // PDMATH_TYPE_TRAITS_H_
