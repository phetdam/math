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

public:
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
    constraint_t<is_value_addressable_v<I>> = 0) noexcept
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
      is_equality_comparable_with_v<I1, I2> > = 0) noexcept
    : data_{&*it}, size_{}
  {
    // TODO: if iterator is random access we can do end - it
    while (!(it == end)) {
      it++;
      size_++;
    }
  }

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

}  // namespace pdmath

#endif  // PDMATH_SPAN_H_
