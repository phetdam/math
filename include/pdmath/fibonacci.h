/**
 * @file fibonacci.h
 * @author Derek Huang
 * @brief C++ header for a Fibonacci sequence generator
 * @copyright MIT License
 */

#ifndef PDMATH_FIBONACCI_H_
#define PDMATH_FIBONACCI_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <utility>

#include "pdmath/features.h"

namespace pdmath {

/**
 * Basic infinite sequence generator for a Fibonacci sequence.
 *
 * This could be used to model an infinite Fibonacci number sequence starting
 * from zero, e.g. 0, 1, 1, 2, 3, 5, ... and so on. Although this is of limited
 * practical interest, it is useful as an example of a *LegacyIterator* without
 * the additional *LegacyInputIterator* type requirements.
 */
class basic_fibonacci_generator {
public:
  /**
   * Yield current Fibonacci number.
   */
  constexpr auto& operator*() const noexcept
  {
    return cur_;
  }

  /**
   * Advance state to the next Fibonacci number.
   *
   * This is done by replacing the previous number with the current one and
   * incrementing the current number with the old previous value.
   *
   * @note When constructed, the Fibonacci iterator has previous value of 1,
   *  as this makes it convenient to start the sequence as 0, 1, 1, ... without
   *  any extra conditional logic to check for zero.
   */
  constexpr auto& operator++() noexcept
  {
    auto old = cur_;
    cur_ += prev_;
    prev_ = old;
    return *this;
  }

// for derived class convenience
protected:
  // note: Fibonacci numbers can get quite big, e.g. F(49) (the 50th number) is
  // 7778742049 and requires more than 32 bits to store. not all implementations
  // provide fixed-width types (although most do) so we use uint_fast64_t
  std::uint_fast64_t prev_{1u};
  std::uint_fast64_t cur_{};
};

/**
 * Infinite sequence generator for a Fibonacci sequence.
 *
 * This models the Fibonacci sequence 0, 1, 1, 2, 3, 5, ... and satisfies the
 * named requirements for a *LegacyInputIterator*. Although still of limited
 * practical use, this should be preferred over `basic_fibonacci_generator`,
 * which was originally intended for type traits testing.
 */
class fibonacci_generator : public basic_fibonacci_generator {
public:
  // inherit operator++ from base
  using basic_fibonacci_generator::operator++;

  /**
   * Compare for equality with another `fibonacci_generator`.
   *
   * @param other Fibonacci generator to compare with
   */
  constexpr bool operator==(const fibonacci_generator& other) const noexcept
  {
    return prev_ == other.prev_ && cur_ == other.cur_;
  }

// in C++20 operator!= is synthesized from operator== and need not be defined
#if !PDMATH_HAS_CXX20
  /**
   * Compare for inequality with another `fibonacci_generator`.
   *
   * @note When compiling with C++20 this is not provided as in C++20 the
   *  definition for `operator!=` is synthesized from `operator==`.
   *
   * @param other Fibonacci generator to compare with
   */
  constexpr bool operator!=(const fibonacci_generator& other) const noexcept
  {
    return !(*this == other);
  }
#endif  // !PDMATH_HAS_CXX20

  /**
   * Advance state to the next Fibonacci number.
   *
   * This supports the post-increment required by *LegacyInputIterator*.
   */
  constexpr auto operator++(int) noexcept
  {
    auto prev = *this;
    ++(*this);
    return prev;
  }

  /**
   * Provide member access to the yielded Fibonacci number.
   *
   * This is not helpful in practice but is required to satisfy the
   * *LegacyInputIterator* named requirements for value member access.
   */
  constexpr auto operator->() const noexcept
  {
    return &cur_;
  }
};

namespace detail {

/**
 * Fibonacci sequence generation template implementation.
 *
 * This handle the `std::index_sequence<Is...>` deduction needed.
 *
 * @tparam T `std::index_sequence<Is...>`
 */
template <typename T>
struct fibonacci_sequence_impl {};

/**
 * Specialization for `std::make_index_sequence<0u>`.
 */
template <>
struct fibonacci_sequence_impl<std::make_index_sequence<0u>> {
  static constexpr std::array<std::uint_fast64_t, 1u> value{0u};
};

/**
 * Partial specialization for `std::make_index_sequence<1u>`.
 */
template <>
struct fibonacci_sequence_impl<std::make_index_sequence<1u>> {
  static constexpr std::array<std::uint_fast64_t, 2u> value{0u, 1u};
};

/**
 * Partial specialization for `std::make_index_sequence<N>`.
 *
 * @tparam I First index (zero)
 * @tparam Is Index pack 1, ... `sizeof...(Is)` (N - 1)
 */
template <std::size_t I, std::size_t... Is>
struct fibonacci_sequence_impl<std::index_sequence<I, Is...>> {
private:
  // previous sequence type
  // note: need to subtract 1 so indices are 0, ... sizeof...(Is) - 1
  using prev = fibonacci_sequence_impl<std::index_sequence<(Is - 1u)...>>;
public:
  static constexpr std::array value{
    prev::value[0],      // F(0)
    prev::value[Is]...,  // F(1) through F(sizeof...(Is))
    // F(sizeof...(Is)) + F(sizeof...(Is) - 1) is F(N - 1) + F(N - 2)
    prev::value[sizeof...(Is)] + prev::value[sizeof...(Is) - 1u]
  };
};

}  // namespace detail

/**
 * Fibonacci sequence generation template.
 *
 * This generates the first `N` Fibonacci numbers 0, 1, 1, 2, 3, ... and stores
 * them in an array, providing a way to reference the numbers at compile-time.
 * The number of elements in the array is `N + 1` for all `N`.
 *
 * @note By defining `N` as the *index* of the largest Fibonacci number we can
 *  one-to-map to the OEIS list of the first 2000 Fibonacci numbers, which can
 *  be found here: https://oeis.org/A000045/b000045.txt
 *
 * @tparam N Zero index of the largest Fibonacci number to generate
 */
template <std::size_t N>
struct fibonacci_sequence
  : detail::fibonacci_sequence_impl<std::make_index_sequence<N>> {};

/**
 * Stream the `fibonacci_sequence<N>` object to an output stream.
 *
 * @tparam N Zero index of the largest Fibonacci number to generate
 *
 * @param out Stream to write to
 */
template <std::size_t N>
auto& operator<<(std::ostream& out, fibonacci_sequence<N> /*seq*/)
{
  using sequence = fibonacci_sequence<N>;
  // for N, the value array will contain N + 1 values for F(0), ... F(N)
  out << "[" << sequence::value[0];
  // print the rest with delimiters
  for (std::size_t i = 1u; i < N + 1u; i++)
    out << ", " << sequence::value[i];
  // add trailing delimiter and finish
  return out << "]";
}

}  // namespace pdmath

#endif  // PDMATH_FIBONACCI_H_
