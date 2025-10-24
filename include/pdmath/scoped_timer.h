/**
 * @file scoped_timer.h
 * @author Derek Huang
 * @brief C++ header for a scoped timer
 * @copyright MIT License
 */

#ifndef PDMATH_SCOPED_TIMER_H_
#define PDMATH_SCOPED_TIMER_H_

#include <chrono>

namespace pdmath {

/**
 * RAII scoped timer class.
 *
 * @todo Add traits type that checks if `clock::duration` can be converted to
 *  `T` via `std::chrono::duration_cast` so we have type checking
 *
 * @tparam T `std::chrono::duration`
 */
template <typename T>
class scoped_timer {
public:
  using clock = std::chrono::steady_clock;
  using duration = T;
  using time_point = clock::time_point;

  /**
   * Ctor.
   *
   * On constructtion the starting time point is set.
   *
   * @param out Duration value to update on scope exit
   */
  scoped_timer(T& out) noexcept : start_{clock::now()},  out_{out} {}

  /**
   * Dtor.
   *
   * On destruction the duration reference is updated.
   */
  ~scoped_timer()
  {
    // note: technically not noexcept
    out_ = std::chrono::duration_cast<T>(clock::now() - start_);
  }

private:
  time_point start_;
  T& out_;  // reference makes type non-copyable
};

}  // namespace pdmath

#endif  // PDMATH_SCOPED_TIMER_H_
