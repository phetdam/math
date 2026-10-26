/**
 * @file akima_spline.h
 * @author Derek Huang
 * @brief C++ header for the Akima cubic spline
 * @copyright MIT License
 */

#ifndef PDMATH_AKIMA_SPLINE_H_
#define PDMATH_AKIMA_SPLINE_H_

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * Invocable class representing the Akima cubic spline.
 *
 * This class uses Akima's original "natural" looking cubic spline construction
 * method that relies only on local points and imposes conditions only on first
 * derivatives. The first and last two curve endpoints rely on quadratic
 * extrapolation of the knot interval slopes using the formulas given in
 * https://blogs.mathworks.com/cleve/2019/04/29/makima-piecewise-cubic-interpolation/.
 *
 * A user-defined deduction guide is provided to mostly eliminate the need to
 * explicitly specify the template type parameter.
 *
 * @tparam T Floating-point type
 */
template <typename T, typename = constraint_t<std::is_floating_point_v<T>>>
class akima_spline {
private:
  /**
   * Empty tag type for ctor overload disambiguation.
   *
   * @note We define our own tag type to avoid including `<variant>`.
   */
  struct range_ctor_tag {};

  /**
   * Indicate that a type can be used as a ctor input range value type.
   *
   * @tparam U type
   */
  template <typename U>
  static constexpr bool valid_type = std::is_convertible_v<U, T>;

  /**
   * Indicate that a range can be used as a ctor input.
   *
   * @tparam R Range type
   */
  template <typename R>
  static constexpr bool valid_range = std::is_convertible_v<range_value_t<R>, T>;

  /**
   * Proxy class representing the spline's derivative.
   *
   * The scope of the derivative object is tied to the spline's own scope.
   *
   * @tparam I Order of the derivative
   */
  template <int I, typename = constraint_t<(I > 0)>>
  class derivative {
  public:
    /**
     * Ctor.
     *
     * @param f Akima spline
     */
    derivative(const akima_spline& f) noexcept : f_{&f} {}

    /**
     * Evaluate the Akima spline's derivative at the given point.
     *
     * @param v Evaluation point
     */
    auto operator()(T v) const noexcept
    {
      // higher derivatives are 0
      if constexpr (I > 3)
        return T{};
      // nonzero derivatives
      else {
        // references to knot points, knot values, and knot derivatives
        auto& xs = f_->xs_;
        auto& ys = f_->ys_;
        auto& ss = f_->ss_;
        // determine bucket. i will be in {0, ... xs.size() - 2}
        auto i = 0u;
        while ((i + 2 < xs.size()) && (v > xs[i + 1]))
          i++;
        // compute interval length, interval slope, and v - xs[i]
        auto dx = xs[i + 1] - xs[i];
        auto m = (ys[i + 1] - ys[i]) / dx;
        auto dv = v - xs[i];
        // compute polynomial coefficients
        // note: don't need a = ys[i] as constant is never used in derivatives
        auto b = ss[i];
        auto c = (3 * m - 2 * ss[i] - ss[i + 1]) / dx;
        auto d = (ss[i] + ss[i + 1] - 2 * m) / dx / dx;
        // third derivative
        if constexpr (I == 3)
          return 6 * d;
        // second derivative
        else if constexpr (I == 2)
          return 2 * c + 6 * d * dv;
        // first derivative
        else
          return b + 2 * c * dv + 3 * d * dv * dv;
      }
    }

    /**
     * Return the proxy object representing a higher-order spline derivative.
     *
     * @tparam I Order of the derivative
     */
    template <int I_ = 1, typename = constraint_t<(I_ > 0)>>
    auto d() const noexcept
    {
      return derivative<I + I_>{*f_};
    }

  private:
    const akima_spline* f_;
  };

public:
  /**
   * Default ctor.
   *
   * This is provided for convenience as nothing interesting can be done with a
   * defaulted instance of the `akima_spline`.
   */
  akima_spline() = default;

  /**
   * Ctor.
   *
   * Constructs from two initializer lists with convertible values.
   *
   * @param xs Knot points
   * @param ys Knot values
   */
  template <typename T1, typename T2>
  akima_spline(
    std::initializer_list<T1> xs,
    std::initializer_list<T2> ys,
    constraint_t<valid_type<T1> && valid_type<T2>> = 0)
    : akima_spline{xs, ys, range_ctor_tag{}}
  {}

  /**
   * Ctor.
   *
   * Constructs from two ranges of the same length with convertible values.
   *
   * @param xs Knot points
   * @param ys Knot values
   */
  template <typename R1, typename R2>
  akima_spline(
    R1&& xs,
    R2&& ys,
    range_ctor_tag = {},
    constraint_t<valid_range<R1> && valid_range<R2>> = 0)
  {
    // need at least 4 knot points
    auto n = std::size(xs);
    if (n < 4)
      throw std::runtime_error{"at least 4 knot points required"};
    // size check
    if (auto ny = std::size(ys); n != ny)
      throw std::runtime_error{
        "number of knot points " + std::to_string(n) +
        " != number of knot values " + std::to_string(ny)
      };
    // knot points must be increasing
    if (!std::is_sorted(std::begin(xs), std::end(xs)))
      throw std::runtime_error{"knot points are not monotone increasing"};
    // allocate + copy to xs_, ys_
    xs_ = decltype(xs_)(n);
    ys_ = decltype(ys_)(n);
    std::copy(std::begin(xs), std::end(xs), xs_.begin());
    std::copy(std::begin(ys), std::end(ys), ys_.begin());
    // compute the n - 1 slope differences across each (x, y) pair + an extra 4
    // quadratically extrapolated slope differences, 2 prepended, 2 appended
    std::vector<T> ms(n + 3);
    // compute ms[2] through ms[n] using xs_, ys_ values
    for (auto i = 0u; i < n - 1; i++)
      ms[i + 2] = (ys_[i + 1] - ys_[i]) / (xs_[i + 1] - xs_[i]);
    // compute the 4 extrapolated endpoint slopes using the formulas from
    // https://blogs.mathworks.com/cleve/2019/04/29/makima-piecewise-cubic-interpolation/
    ms[n + 1] = 2 * ms[n] - ms[n - 1];
    ms[n + 2] = 2 * ms[n + 1] - ms[n];
    ms[1] = 2 * ms[2] - ms[3];
    ms[0] = 2 * ms[1] - ms[2];
    // allocate + compute the ss_ spline slopes
    ss_ = decltype(ss_)(n);
    for (auto i = 0u; i < n; i++) {
      // forward + backward absolute differences of ms
      // note: ms indices offset by 2 due to the extra extrapolated points
      auto w1 = (std::abs)(ms[i + 3] - ms[i + 2]);
      auto w2 = (std::abs)(ms[i + 1] - ms[i]);
      // if w1 + w2 is small simply average the ms out
      if ((w1 + w2) <= 10000 * std::numeric_limits<T>::epsilon())
        ss_[i] = (ms[i + 1] + ms[i + 2]) / 2;
      // otherwise use full Akima weighted average
      else
        ss_[i] = (w1 * ms[i + 1] + w2 * ms[i + 2]) / (w1 + w2);
    }
  }

  /**
   * Evaluate the Akima spline at the given point.
   *
   * If the point is outside the interpolation interval the corresponding
   * endpoint cubic polynomial will be used for extrapolation.
   *
   * @param v Evaluation point
   */
  auto operator()(T v) const noexcept
  {
    // determine bucket. i will be in {0, ... xs_.size() - 2}
    auto i = 0u;
    while ((i + 2 < xs_.size()) && (v > xs_[i + 1]))
      i++;
    // compute interval width + slope over the interval
    auto dx = xs_[i + 1] - xs_[i];
    auto m = (ys_[i + 1] - ys_[i]) / dx;
    // compute v - xs_[i] powers
    auto dv = v - xs_[i];
    auto dv2 = dv * dv;
    auto dv3 = dv2 * dv;
    // compute polynomial coefficients
    auto a = ys_[i];
    auto b = ss_[i];
    auto c = (3 * m - 2 * ss_[i] - ss_[i + 1]) / dx;
    auto d = (ss_[i] + ss_[i + 1] - 2 * m) / dx / dx;
    // evaluate
    return a + b * dv + c * dv2 + d * dv3;
  }

  /**
   * Return the derivative of the Akima spline.
   *
   * @tparam I Order of the derivative
   */
  template <int I = 1, typename = constraint_t<(I > 0)>>
  auto d() const noexcept
  {
    return derivative<I>{*this};
  }

private:
  std::vector<T> xs_;  // knot points
  std::vector<T> ys_;  // knot values
  std::vector<T> ss_;  // knot point first derivatives
};

/**
 * User-defined deduction guide for the `akima_spline`.
 *
 * This deduces the template parameter to the wider of the range value types.
 *
 * @tparam R1 Input range
 * @tparam R2 Input range
 */
template <typename R1, typename R2>
akima_spline(R1&&, R2&&) -> akima_spline<
  std::common_type_t<range_value_t<R1>, range_value_t<R2>>
>;

/**
 * User-defined deduction guide for the `akima_spline`.
 *
 * This deduces the template parameter to the wider of the initializer list
 * value types and is provided separately due to template deduction rules.
 *
 * @tparam T1 Floating-point type
 * @tparam T2 Floating-point type
 */
template <typename T1, typename T2>
akima_spline(
  std::initializer_list<T1>,
  std::initializer_list<T2>) -> akima_spline<std::common_type_t<T1, T2>>;

}  // namespace pdmath

#endif  // PDMATH_AKIMA_SPLINE_H_
