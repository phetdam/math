/**
 * @file type_traits.h
 * @author Derek Huang
 * @brief Extensions to the C++ standard library `<type_traits>` header.
 * @copyright MIT License
 */

#ifndef PDMATH_TYPE_TRAITS_H_
#define PDMATH_TYPE_TRAITS_H_

#include <type_traits>

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

#endif  // PDMATH_TYPE_TRAITS_H_
