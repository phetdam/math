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
inline constexpr bool
is_vector_size_constructible_v = is_vector_size_constructible<T>::value;

/**
 * Checks if a type is castable to `std::string`.
 *
 * That is, the type `T` implements or inherits `operator std::string`, which
 * may or may not be declared as explicit, i.e. cast required.
 *
 * @param T type
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
inline constexpr bool has_operator_string_v = has_operator_string<T>::value;

}  // namespace

#endif  // PDMATH_TYPE_TRAITS_H_
