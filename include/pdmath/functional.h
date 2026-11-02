/**
 * @file functional.h
 * @author Derek Huang
 * @brief C++ header for general functional algorithms
 * @copyright MIT License
 */

#ifndef PDMATH_FUNCTIONAL_H_
#define PDMATH_FUNCTIONAL_H_

#include <type_traits>

#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * CPO type for returning the minimum element in a range.
 *
 * This is a wrapper over `std::min_element()` for C++17.
 */
struct min_of_cpo {
  /**
   * Return an iterator to the minimum element in a range.
   *
   * The elements of the range must be inequality comparable.
   *
   * @tparam T Range type
   *
   * @param vs Range of values
   */
  template <typename T>
  constexpr auto operator()(
    T&& vs,
    constraint_t<is_less_than_supported_v<range_value_t<T>> > = 0) const
  {
    using std::begin;
    using std::end;
    return std::min_element(begin(vs), end(vs));
  }
};

/**
 * Customization point object for returning the minimum element in a range.
 */
constexpr min_of_cpo min_of{};

/**
 * CPO type for returning the maximum element in a range.
 *
 * This is a wrapper over `std::max_element()` for C++17.
 */
struct max_of_cpo {
  /**
   * Return an iterator to the maximum element in a range.
   *
   * The elements in the range must be inequality comparable.
   *
   * @tparam T Range type
   */
  template <typename T>
  constexpr auto operator()(
    T&& vs,
    constraint_t<is_less_than_supported_v<range_value_t<T>> > = 0) const
  {
    using std::begin;
    using std::end;
    return std::max_element(begin(vs), end(vs));
  }
};

/**
 * Customization point object for returning the maximum element in a range.
 */
constexpr max_of_cpo max_of{};

/**
 * CPO type for invoking a callable on each element of a range.
 *
 * This is a wrapper of `std::for_each()` for C++17.
 */
struct for_each_cpo {
  /**
   * Invoke the unary callable on each element of the range.
   *
   * @note It is required that the callable must operate on a reference.
   *
   * @tparam T Range type
   * @tparam F Unary invocable
   *
   * @param vs Range of values
   * @param f Unary invocable to apply
   */
  template <typename T, typename F>
  constexpr auto operator()(
    T&& vs,
    F&& f,
    constraint_t<std::is_invocable_v<F, range_value_t<T>&> > = 0) const
  {
    using std::begin;
    using std::end;
    return std::for_each(begin(vs), end(vs), std::forward<F>(f));
  }
};

/**
 * Customization point object for invoking a callable on each range element.
 */
constexpr for_each_cpo for_each{};

}  // namespace pdmath

#endif  // PDMATH_FUNCTIONAL_H_
