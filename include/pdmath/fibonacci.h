/**
 * @file fibonacci.h
 * @author Derek Huang
 * @brief C++ header for a Fibonacci sequence generator
 * @copyright MIT License
 */

#ifndef PDMATH_FIBONACCI_H_
#define PDMATH_FIBONACCI_H_

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
  auto& operator*() const noexcept
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
  auto& operator++() noexcept
  {
    auto old = cur_;
    cur_ += prev_;
    prev_ = old;
    return *this;
  }

// for derived class convenience
protected:
  unsigned prev_{1u};
  unsigned cur_{};
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
  bool operator==(const fibonacci_generator& other) const noexcept
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
  bool operator!=(const fibonacci_generator& other) const noexcept
  {
    return !(*this == other);
  }
#endif  // !PDMATH_HAS_CXX20

  /**
   * Advance state to the next Fibonacci number.
   *
   * This supports the post-increment required by *LegacyInputIterator*.
   */
  auto operator++(int) noexcept
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
  auto operator->() const noexcept
  {
    return &cur_;
  }
};

}  // namespace pdmath

#endif  // PDMATH_FIBONACCI_H_
