/**
 * @file span.h
 * @author Derek Huang
 * @brief C++ header for a view type over a contiguous buffer
 * @copyright MIT License
 */

#ifndef PDMATH_SPAN_H_
#define PDMATH_SPAN_H_

#include <cstddef>
#include <type_traits>
#include <utility>

#include "pdmath/type_traits.h"

namespace pdmath {

// forward decl for traits
template <typename T>
class span;

// TODO: not yet sure if is_span<T> should be exposed in top-level namespace
namespace detail {

/**
 * Traits to check if the type is a `span<T>`.
 *
 * @tparam T type
 */
template <typename T>
struct is_span : std::false_type {};

/**
 * True specialization for `span<T>`.
 *
 * @tparam T type
 */
template <typename T>
struct is_span<span<T>> : std::true_type {};

}  // namespace detail

/**
 * Class representing a view over a contiguous buffer.
 *
 * This provides a C++17 analogue to the C++20 `std::span` with dynamic extent.
 *
 * @tparam T type
 */
template <typename T>
class span {
private:
  /**
   * Traits indicating a type with an addressable value.
   *
   * @tparam I type
   */
  template <typename I, typename = void>
  struct is_value_addressable : std::false_type {};

  /**
   * True specialization for a type with an addressable value.
   *
   * @tparam I type
   */
  template <typename I>
  struct is_value_addressable<
    I,
    std::enable_if_t<std::is_pointer_v<decltype(&*std::declval<I>())>> >
    : std::true_type {};

  /**
   * Indicate that a type has an addressable value.
   *
   * @tparam I type
   */
  template <typename I>
  static constexpr bool is_value_addressable_v = is_value_addressable<I>::value;

  /**
   * Traits indicating a reference type is compatible with `T`.
   *
   * The value type must either be `T` or `T` without `const`, or in the case of
   * pointers, be a pointer with or without the const-qualification of `T`.
   *
   * @tparam U type
   */
  template <typename U, typename = void>
  struct is_compatible_reference : std::false_type {};

  /**
   * True specialization for a reference type compatible with `T`.
   *
   * @tparam U type
   */
  template <typename U>
  struct is_compatible_reference<
    U,
    std::enable_if_t<
      std::is_same_v<T, std::remove_reference_t<U>> ||
      std::is_same_v<std::remove_const_t<T>, std::remove_reference_t<U>> ||
      std::is_same_v<
        std::remove_const_t<std::remove_pointer_t<T>>,
        std::remove_pointer_t<std::remove_reference_t<U>>
      >
    > > : std::true_type {};

  /**
   * Indicate that a reference type is compatible with `T`.
   *
   * @tparam U type
   */
  template <typename U>
  static constexpr bool
  is_compatible_reference_v = is_compatible_reference<U>::value;

public:
  // note: type members are for compatibility with GoogleMock 1.11.0 which uses
  // value_type and const_iterator when using EXPECT_THAT() with containers
  using value_type = T;
  using iterator = T*;
  using const_iterator = T*;

  /**
   * Default ctor.
   *
   * This constructs an empty span with `nullptr` data.
   */
  constexpr span() noexcept : data_{}, size_{} {}

  /**
   * Ctor.
   *
   * @tparam I Iterator with addressable value
   *
   * @param it Iterator to addressable value
   * @param size Number of elements in span
   */
  template <typename I>
  constexpr span(
    I it,
    std::size_t size,
    constraint_t<
      is_value_addressable_v<I> &&
      is_compatible_reference_v<iter_reference_t<I>> > = 0)
    : data_{&*it}, size_{size}
  {}

  /**
   * Ctor.
   *
   * @tparam I1 Iterator with addressable value
   * @tparam I2 Iterator or sentinel type
   *
   * @param it Input iterator to addressable value
   * @param end End iterator or sentinel type
   */
  template <typename I1, typename I2>
  constexpr span(
    I1 it,
    I2 end,
    constraint_t<
      is_legacy_iterator_v<I1> &&
      is_value_addressable_v<I1> &&
      is_compatible_reference_v<iter_reference_t<I1>> &&
      is_equality_comparable_with_v<I1, I2> > = 0)
    : data_{&*it}, size_{}
  {
    // if end - it is supported use that to compute size
    if constexpr (is_subtraction_supported_with_v<I2, I1>)
      size_ = static_cast<std::size_t>(end - it);
    // otherwise advance it until end to compute size_
    else {
      while (!(it == end)) {
        ++it;
        size_++;
      }
    }
  }

  /**
   * Ctor.
   *
   * Construct from a range that is not another span.
   *
   * @tparam R Range type
   *
   * @param r Range object
   */
  template <typename R>
  constexpr span(
    R&& r,
    constraint_t<
      !detail::is_span<R>::value &&
      is_range_v<R> &&
      is_compatible_reference_v<range_reference_t<R>> > = 0)
    : span{std::begin(r), std::end(r)}
  {}

  /**
   * Ctor.
   *
   * Construct from a reference to a C array.
   *
   * @note Unlike `std::span` we allow CTAD and don't require that the array's
   *  value type be in a non-deduced context as `span<T>` has no fixed size.
   *
   * @tparam N Number of elements in array
   *
   * @param ar Array
   */
  template <std::size_t N>
  constexpr span(T (&ar)[N]) noexcept : span{&*ar, N} {}

  /**
   * Ctor.
   *
   * Construct from another `span` without the same const-qualification.
   *
   * @tparam U `T` without const-qualification
   *
   * @param s Other `span<U>` to construct from
   */
  template <typename U>
  constexpr span(
    const span<U>& s,
    constraint_t<std::is_same_v<T, const U>> = 0) noexcept
    : data_{s.data_}, size_{s.size_}
  {}

  /**
   * Return the data pointer.
   */
  constexpr auto data() const noexcept { return data_; }

  /**
   * Return the size of the span.
   */
  constexpr auto size() const noexcept { return size_; }

  /**
   * Return a reference to the specified element.
   *
   * @param i Element index
   */
  constexpr auto& operator[](std::size_t i) const noexcept
  {
    return data_[i];
  }

  /**
   * Return an iterator to the first element.
   */
  constexpr auto begin() const noexcept
  {
    return data_;
  }

  /**
   * Return an iterator to one past the last element.
   */
  constexpr auto end() const noexcept
  {
    return data_ + size_;
  }

private:
  T* data_;
  std::size_t size_;
};

/**
 * User-defined deduction guide.
 *
 * This ensures that we deduce the correct value type from the iterator.
 *
 * @tparam I Iterator type
 */
template <typename I>
span(I, std::size_t) -> span<std::remove_reference_t<iter_reference_t<I>>>;

/**
 * User-defined deduction guide.
 *
 * This ensures that we deduce the correct value type from the iterator.
 *
 * @tparam I1 Iterator type
 * @tparam I2 Iterator or sentinel type
 */
template <typename I1, typename I2>
span(I1, I2) -> span<std::remove_reference_t<iter_reference_t<I1>>>;

/**
 * User-defined deduction guide.
 *
 * This ensures that we deduce the correct value type from the range.
 *
 * @tparam R Range type
 */
template <typename R>
span(R&&) -> span<std::remove_reference_t<range_reference_t<R>>>;

}  // namespace pdmath

#endif  // PDMATH_SPAN_H_
