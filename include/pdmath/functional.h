/**
 * @file functional.h
 * @author Derek Huang
 * @brief C++ header for general functional algorithms
 * @copyright MIT License
 */

#ifndef PDMATH_FUNCTIONAL_H_
#define PDMATH_FUNCTIONAL_H_

#include <type_traits>
#include <utility>

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
 * This is a reimplementation of `std::for_each()` with reduced requirements on
 * the iterator and invocable. Furthermore, the `operator()` is `constexpr`.
 * Prefer using this over `std::for_each()` due to this increased flexibility.
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
    // note: it need only be a LegacyIterator and st can be a sentinel type
    auto it = begin(vs);
    auto st = end(vs);
    // note: only operator== needs implementation
    while (!(it == st)) {
      f(*it);
      ++it;
    }
    // forwarded
    return std::forward<F>(f);
  }
};

/**
 * Customization point object for invoking a callable on each range element.
 */
constexpr for_each_cpo for_each{};

/**
 * CPO type for transforming each element of a range and writing to another.
 *
 * This is a `constexpr` capable reimplementation of `std::transform()` with
 * reduced requirements on the iterator and invocable. Prefer using this over
 * `std::transform()` due to this increased flexibility.
 */
struct transform_cpo {
  /**
   * SFINAE helper for the `operator()` constraint.
   *
   * @tparam F Unary invocable
   * @tparam T Input range/iterator type
   * @tparam U Output range/iterator type
   */
  template <typename F, typename T, typename U>
  using invoke_constraint_t = constraint_t<
    std::is_convertible_v<std::invoke_result_t<F, T&>, U>
  >;

  /**
   * Invoke the unary callable on each element of the range.
   *
   * @note It is required that the callable must operate on a reference.
   *
   * @tparam T Range type
   * @tparam O Output iterator type
   * @tparam F Unary invocable
   *
   * @param vs Range of values
   * @param out Output iterator
   * @param f Unary invocable to apply
   */
  template <typename T, typename O, typename F>
  constexpr auto operator()(
    T&& vs,
    O out,
    F&& f,
    invoke_constraint_t<F, range_value_t<T>, iter_value_t<O>> = 0) const
  {
    using std::begin;
    using std::end;
    // note: it need only be a LegacyIterator and st can be a sentinel type
    auto it = begin(vs);
    auto st = end(vs);
    // note: only operator== needs implementation
    while (!(it == st)) {
      *out = f(*it);
      ++it;
      ++out;
    }
    // return sentinel/iterator
    return std::move(out);
  }

  // TODO: add operator() for two ranges
};

/**
 * Customization point object for transforming range elements to another range.
 */
inline constexpr transform_cpo transform{};

}  // namespace pdmath

#endif  // PDMATH_FUNCTIONAL_H_
