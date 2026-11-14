/**
 * @file type_traits.h
 * @author Derek Huang
 * @brief C++ header for additional type traits
 * @copyright MIT License
 */

#ifndef PDMATH_TYPE_TRAITS_H_
#define PDMATH_TYPE_TRAITS_H_

#include <cstddef>
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
 * @deprecated Just use `is_convertible_v<T, std::string>` instead.
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
 * Constraint scoped enumeration type.
 *
 * This replaces the older usage of `constraint_t<true>` being an alias for
 * `int` to prevent overload ambiguity with `constraint_t` as a trailing
 * defaulted function parameter since integral types cannot implicitly convert
 * to scoped enumeration types. However, scoped enumeration types can still be
 * used as non-type template parameters, which preserves the ability to use
 * `constraint_t` as both a type and non-type template parameter.
 *
 * The `value` member is provided to enable value-initialization.
 */
enum class constraint_values : int { zero = 0 };

/**
 * Constraint scoped enumeration global.
 *
 * This is a less verbose initializer for `constraint_t<V>` when
 * `auto = constraint_t<V>{}` cannot be used due to redefinition issues. The
 * reason `constraint_t<V> = constraint_v` works is because the evaluation of
 * `V` to `false` will cause substitution failure but not an error. However,
 * using `auto` as a placeholder type results in a template redefinition.
 */
inline constexpr auto constraint_v = constraint_values::zero;

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
 * In addition, because the `constraint_t` SFINAE helper provides a scoped
 * enumeration type member that cannot be implicitly converted to from `int`,
 * `constraint_t<V> = {}` can be passed as a trailing parameter to constrain a
 * function template without the risk of overload ambiguity. However, the
 * downside is that `constraint_t<V> = {}` *cannot* be used as a non-type type
 * parameter due to current C++ grammar rules. You should initialize the
 * `constraint_t<T>` template type (do *not* use `auto` to avoid template
 * redefinition) with `constraint_v` instead. CWG2450 addresses this defect:
 * https://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#2450.
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
 * auto float_only(T v, constraint_t<std::is_floating_point_v<T>> = {})
 * {
 *   return v + 1;
 * }
 * @endcode
 *
 * In cases where an optional argument is not allowed or not desirable, one can
 * use it like `std::enable_if_t` as a template type parameter:
 *
 * @code{.cc}
 * template <typename T, typename = constraint_t<std::is_floating_point_v<T>> >
 * auto float_only(T v)
 * {
 *   return v + 1;
 * }
 * @endcode
 *
 * Do note that this usage is discouraged, as end-users can bypass the template
 * constraint by simply explicitly passing a type through. For example:
 *
 * @code{.cc}
 * float_only<unsigned, void>(2);
 * @endcode
 *
 * Therefore, prefer to use a non-type template parameter as follows:
 *
 * @code{.cc}
 * template <typename T, constraint_t<std::is_integral_v<T>> = constraint_v>
 * auto integral_only(T v)
 * {
 *   return v + 1;
 * }
 * @endcode
 *
 * This form is required as `{}` cannot be directly used as the initializer as
 * described in the `constraint<V>` documentation; the defect is addressed by
 * CWG2450. If `auto` is used, one can end up with template redefinition, which
 * is avoided when using `constraint_t<V>` directly to trigger SFINAE. Also,
 * `auto` can be bypassed with an unrelate value like when using `typename`.
 *
 * @tparam V `constexpr` truth value
 */
template <bool V>
using constraint_t = typename constraint<V>::type;

/**
 * True specialization that provides the `constraint_values` type alias.
 *
 * As mentioned before the use of `constraint_values` enables `constraint_t` to
 * be used as both a type and non-type template parameter under the constraints
 * of C++17 while also avoiding overload ambiguities when `constraint_t` is
 * used as a defaulted, trailing function parameter.
 */
template <>
struct constraint<true> {
  using type = constraint_values;
};

/**
 * Traits type that always has a `false` value member.
 *
 * This is useful for representing false when testing and is essentially
 * `std::false_type` but parameterized with a type parameter.
 *
 * @tparam T type
 */
template <typename T>
struct always_false : std::false_type {};

/**
 * Traits type that always has a `true` value member.
 *
 * This is useful for representing true when testing and is essentially
 * `std::true_type` but parameterized with a type parameter.
 *
 * @tparam T type
 */
template <typename T>
struct always_true : std::true_type {};

/**
 * Traits type that provides the same type passed as its input.
 *
 * This should be used to establish non-deduced contexts.
 *
 * @tparam T type
 */
template <typename T>
struct type_identity {
  using type = T;
};

/**
 * SFINAE helper for the type identity traits for a non-deduced context.
 *
 * @tparam T type
 */
template <typename T>
using type_identity_t = typename type_identity<T>::type;

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
 * @note We force use of lvalue references here because a range technically
 *  needs to be in scope for its iterators to work.
 *
 * @tparam T type
 */
template <typename T>
struct is_range<
  T,
  std::void_t<decltype(std::begin(std::declval<T&>()))>,
  std::void_t<decltype(std::end(std::declval<T&>()))>
> : std::true_type {};

/**
 * Indicate that a type is a range-like type.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_range_v = is_range<T>::value;

/**
 * Traits type for an indirectly readable type.
 *
 * That is, the type of an object `x` that supports the `*x` expression. Note
 * that `*x` need not yield a reference, but that `x` itself must be an lvalue.
 * This satisfies the definition of `std::iter_reference_t<T>` as used in the
 * definition of the `std::indirectly_readable` concept.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_indirectly_readable : std::false_type {};

/**
 * True specialization for types that support dereferencing.
 *
 * @tparam T type
 */
template <typename T>
struct is_indirectly_readable<T, std::void_t<decltype(*std::declval<T&>())> >
  : std::true_type {};

/**
 * Indicate that a type is indirectly readable (can be dereferenced).
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_indirectly_readable_v = is_indirectly_readable<T>::value;

/**
 * Traits type for a weakly incrementable type.
 *
 * This partially satisfies the C++20 `std::weakly_incrementable` concept. In
 * particular, there are no constraints on movability, although we do require
 * that the pre-increment operation yields a reference.
 *
 * Roughly speaking, this traits checks that a type supports both the pre- and
 * post-increment operations, where pre-increment yields a reference.
 *
 * @tparam T type
 */
template <typename T, typename = void, typename = void>
struct is_weakly_incrementable : std::false_type {};

/**
 * True specialization for a type that is weakly incrementable.
 *
 * @tparam T type
 */
template <typename T>
struct is_weakly_incrementable<
  T,
  // note: need to use T& since increment is defined only on glvalues. we just
  // rely on reference collapsing rules to deal with T being reference
  std::enable_if_t<std::is_reference_v<decltype(++std::declval<T&>())>>,
  std::void_t<decltype(std::declval<T&>()++)>
> : std::true_type {};

/**
 * Indicate that a type is weakly incrementable.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_weakly_incrementable_v = is_weakly_incrementable<T>::value;

/**
 * Traits type for a *LegacyIterator*.
 *
 * This partially satisfies the *LegacyIterator* named requirements. The main
 * difference is that there is no `std::iterator_traits` member requirement.
 *
 * @tparam T type
 */
template <typename T, typename = void, typename = void, typename = void>
struct is_legacy_iterator : std::false_type {};

/**
 * True specialization for a type that satisfies *LegacyIterator* semantics.
 *
 * @tparam T type
 */
template <typename T>
struct is_legacy_iterator<
  T,
  std::enable_if_t<
    std::is_copy_constructible_v<T> &&
    std::is_copy_assignable_v<T> &&
    std::is_destructible_v<T> &&
    std::is_swappable_v<T>
  >,
  // note: LegacyIterator requirements do not specify return type
  std::void_t<decltype(*std::declval<T&>())>,
  // note: as before, need an lvalue reference for pre/post increment
  std::enable_if_t<std::is_reference_v<decltype(++std::declval<T&>())>>
> : std::true_type {};

/**
 * Indicate that a type satisfies *LegacyIterator* semantics.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_legacy_iterator_v = is_legacy_iterator<T>::value;

/**
 * Traits type for the reference type of an iterator-like type.
 *
 * This can be used for non-iterator types as well as the minimum requirement
 * is that `is_indirectly_readable_v<T>` is `true`.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct iter_reference {};

/**
 * True specialization for an iterator-like type.
 *
 * @tparam T type
 */
template <typename T>
struct iter_reference<T, std::enable_if_t<is_indirectly_readable_v<T>> > {
  using type = decltype(*std::declval<T>());
};

/**
 * SFINAE helper for the reference type of an iterator-like type.
 *
 * @note `iter_reference_t<T>` need *not* be a reference type. For example,
 *  some constructs implement `operator*()` that returns a value type.
 *
 * @tparam T type
 */
template <typename T>
using iter_reference_t = typename iter_reference<T>::type;

/**
 * Traits type for the reference type of a range-like type.
 *
 * @tparam T type
 */
template <typename T, typename = void, typename = void>
struct range_reference {};

/**
 * True specialization for a range-like type.
 *
 * @tparam T type
 */
template <typename T>
struct range_reference<
  T,
  std::enable_if_t<is_range_v<T>>,
  std::void_t<iter_reference_t<decltype(std::begin(std::declval<T&>()))>>
> {
  using type = iter_reference_t<decltype(std::begin(std::declval<T&>()))>;
};

/**
 * SFINAE helper for the reference type of a range-like type.
 *
 * @note `range_reference_t<T>` need *not* be a reference type.
 *
 * @tparam T type
 */
template <typename T>
using range_reference_t = typename range_reference<T>::type;

/**
 * Traits type for the value type of an iterator-like type.
 *
 * This can be used for non-iterator types as well as the minimum requirement
 * is that `is_indirectly_readable_v<T>` is `true`.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct iter_value {};

/**
 * True specialization for an iterator-like type.
 *
 * @tparam T type
 */
template <typename T>
struct iter_value<T, std::enable_if_t<is_indirectly_readable_v<T>> > {
  using type = std::decay_t<decltype(*std::declval<T>())>;
};

/**
 * SFINAE helper for the value type of an iterator-like type.
 *
 * @tparam T type
 */
template <typename T>
using iter_value_t = typename iter_value<T>::type;

/**
 * Traits type for the value type of a range-like type.
 *
 * @tparam T type
 */
template <typename T, typename = void, typename = void>
struct range_value {};

/**
 * True specialization for a range-like type.
 *
 * @tparam T type
 */
template <typename T>
struct range_value<
  T,
  std::enable_if_t<is_range_v<T>>,
  std::void_t<iter_value_t<decltype(std::begin(std::declval<T&>()))>>
> {
  using type = iter_value_t<decltype(std::begin(std::declval<T&>()))>;
};

/**
 * SFINAE helper for the value type of a range-like type.
 *
 * @tparam T type
 */
template <typename T>
using range_value_t = typename range_value<T>::type;

/**
 * Traits type for an equality-comparable type.
 *
 * This simply tests that `a == b` is convertible to `bool`.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_equality_comparable : std::false_type {};

/**
 * True specialization for equality-comparable types.
 *
 * @tparam T type
 */
template <typename T>
struct is_equality_comparable<
  T,
  std::enable_if_t<
    std::is_convertible_v<decltype(std::declval<T>() == std::declval<T>()), bool>
  > > : std::true_type {};

/**
 * Indicate that a type is equality-comparable.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;

/**
 * Traits type for testing that two types are equality-comparable.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U, typename = void>
struct is_equality_comparable_with : std::false_type {};

/**
 * True specialization for two types that are equality-comparable.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U>
struct is_equality_comparable_with<
  T,
  U,
  std::enable_if_t<
    std::is_convertible_v<
      decltype(std::declval<T>() == std::declval<U>()), bool>> >
  : std::true_type {};

/**
 * Indicate that two types are equality-comparable.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U>
constexpr bool
is_equality_comparable_with_v = is_equality_comparable_with<T, U>::value;

/**
 * Traits type for a type that is inequality comparable.
 *
 * This simply tests that `a != b` is convertible to `bool`.
 *
 * @note In C++20 `operator!=` can be *synthesized* from `operator==`, which
 *  means that `a == b` implies that `a != b` is also valid.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_inequality_comparable : std::false_type {};

/**
 * True specialization for inequality-comparable types.
 *
 * @tparam T type
 */
template <typename T>
struct is_inequality_comparable<
  T,
  std::enable_if_t<
    std::is_convertible_v<decltype(std::declval<T>() != std::declval<T>()), bool>
  > > : std::true_type {};

/**
 * Indicate that a type is inequality-comparable.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_inequality_comparable_v = is_inequality_comparable<T>::value;

/**
 * Traits type for a *LegacyInputIterator*.
 *
 * This satisfies the *LegacyInputIterator* named requirements.
 *
 * @tparam T type
 */
template <typename T, typename = void, typename = void, typename = void>
struct is_legacy_input_iterator : std::false_type {};

/**
 * True specialization for a type that satisfies *LegacyInputIterator*.
 *
 * @tparam T type
 */
template <typename T>
struct is_legacy_input_iterator<
  T,
  std::enable_if_t<
    is_legacy_iterator_v<T> &&
    is_equality_comparable_v<T> &&
    is_inequality_comparable_v<T> &&  // note: not necessary in C++20
    is_indirectly_readable_v<T>
  >,
  std::enable_if_t<
    !std::is_class_v<std::decay_t<decltype(*std::declval<T&>())>> ||
    !std::is_union_v<std::decay_t<T>> ||
    // note: only applies if value type is not a class or union
    // note: in C++20 input_iterator doesn't require operator->()
    std::is_pointer_v<decltype(std::declval<T>().operator->())>
  >,
  // LegacyInputIterator supports post-increment
  // note: as before, need an lvalue reference for pre/post increment
  std::void_t<decltype(std::declval<T&>()++)>
> : std::true_type {};

/**
 * Indicate that a type satisfies *LegacyInputIterator*.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_legacy_input_iterator_v = is_legacy_input_iterator<T>::value;

// TODO: add is_addition_supported[_v]<T>

/**
 * Traits type for `operator+()` support between two types.
 *
 * Essentially, for `T t`, `U u`, we want to know if `t + u` is supported.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U, typename = void>
struct is_addition_supported_with : std::false_type {};

/**
 * True specialization for two types that support `operator+()`.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U>
struct is_addition_supported_with<
  T, U, std::void_t<decltype(std::declval<T>() + std::declval<U>())> >
  : std::true_type {};

/**
 * Indicate if `operator+()` is supported between the two types.
 *
 * @note `is_addition_supported_with_v<T, U>` may not have the same value as
 *  `is_addition_supported_with_v<U, T>` if `operator+()` is not symmetric.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U>
constexpr bool
is_addition_supported_with_v = is_addition_supported_with<T, U>::value;

// TODO: add is_subtraction_supported[_v]<T>

/**
 * Traits type for `operator-()` support between two types.
 *
 * Essentially, for `T t`, `U u`, we want to know if `t - u` is supported.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U, typename = void>
struct is_subtraction_supported_with : std::false_type {};

/**
 * True specialization for two types that support `operator-()`.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U>
struct is_subtraction_supported_with<
  T, U, std::void_t<decltype(std::declval<T>() - std::declval<U>())> >
  : std::true_type {};

/**
 * Indicate if `operator-()` is supported between the two types.
 *
 * @note `is_subtraction_supported_with_v<T, U>` may not have the same value as
 *  `is_subtraction_supported_with<U, T>` if `operator-()` is not symmetric.
 *
 * @tparam T First type
 * @tparam U Second type
 */
template <typename T, typename U>
constexpr bool
is_subtraction_supported_with_v = is_subtraction_supported_with<T, U>::value;

/**
 * Traits type checking for `operator<()` support.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct is_less_than_supported : std::false_type {};

/**
 * True specialization for a type that supports `operator<()`.
 *
 * @tparam T type
 */
template <typename T>
struct is_less_than_supported<
  T, std::void_t<decltype(std::declval<T>() < std::declval<T>())> >
  : std::true_type {};

/**
 * Indicate if `operator<()` is supported by the given type.
 *
 * @tparam T type
 */
template <typename T>
constexpr bool is_less_than_supported_v = is_less_than_supported<T>::value;

}  // namespace pdmath

#endif  // PDMATH_TYPE_TRAITS_H_
