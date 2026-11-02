/**
 * @file natural_spline.h
 * @author Derek Huang
 * @brief C++ header for a natural cubic spline
 * @copyright MIT License
 */

#ifndef PDMATH_NATURAL_SPLINE_H_
#define PDMATH_NATURAL_SPLINE_H_

#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "pdmath/tridiagonal.h"
#include "pdmath/type_traits.h"
#include "pdmath/warnings.h"

namespace pdmath {

/**
 * Invocable class representing a natural cubic spline.
 *
 * This class models a cubic spline with "natural" boundary conditions and
 * maintains ownership of the knot points, knot values, and the knot point
 * second derivatives. The latter are computed using the Thomas algorithm to
 * solve the tridiagonal system that results from the conditions imposed on the
 * knot point second derivatives. The reference for the implementation is from
 * https://en.wikiversity.org/wiki/Cubic_Spline_Interpolation.
 *
 * A user-defined deduction guide is provided to mostly eliminate the need to
 * explicitly specify the template type parameter.
 *
 * @tparam T Floating-point type
 */
template <typename T, typename = constraint_t<std::is_floating_point_v<T>>>
class natural_spline {
private:
  /**
   * Tag type for ctor overload disambiguation in lieu of `std::monostate`.
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
   * Proxy class representing the natural cubic spline's derivative.
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
     * @param f Parent spline
     */
    derivative(const natural_spline& f) noexcept : f_{&f} {}

    /**
     * Evaluate the natural cubic spline derivative at the given point.
     *
     * If the point is outside the interpolation interval the corresponding
     * endpoint cubic polynomial will be used for extrapolation.
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
        // references to the knot points, knot values, and second derivatives
        auto& xs = f_->xs_;
        auto& ys = f_->ys_;
        auto& ms = f_->ms_;
        // determine bucket. i will be in {0, ... xs_.size() - 2}
        std::size_t i = 0u;
        while ((i + 2 < xs.size()) && (v > xs[i + 1]))
          i++;
        // compute xs[i + 1] - xs[i] difference + 2 * xs[i + 1] - xs[i]
        auto dx = xs[i + 1] - xs[i];
        auto d2x = 2 * dx;
        // evaluate xs[i + 1] - v and v - xs[i] (up and down differences)
        auto dxu = xs[i + 1] - v;
        auto dxd = v - xs[i];
        // third derivative
        if constexpr (I == 3)
          return (ms[i + 1] - ms[i]) / dx;
        // second derivative
        else if constexpr (I == 2)
          return ms[i] * dxu / dx + ms[i + 1] * dxd / dx;
        // first derivative
        else
          return (
            -ms[i] * dxu * dxu / d2x + ms[i + 1] * dxd * dxd / d2x +
            (ys[i + 1] - ys[i]) / dx - (ms[i + 1] - ms[i]) * dx / 6
          );
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
    const natural_spline* f_;
  };

public:
  /**
   * Default ctor.
   *
   * This is provided for convenience, e.g. when deferred initialization is
   * needed. Nothing useful can be done with a default-constructed instance.
   */
  natural_spline() = default;

  /**
   * Ctor.
   *
   * Constructs from initializer lists with convertible values.
   *
   * @tparam T1 Element type
   * @tparam T2 Element type
   *
   * @param xs Knot points
   * @param ys Knot values
   */
  template <typename T1, typename T2>
  natural_spline(
    std::initializer_list<T1> xs,
    std::initializer_list<T2> ys,
    constraint_t<valid_type<T1> && valid_type<T2>> = 0)
    : natural_spline{xs, ys, range_ctor_tag{}}
  {}

  /**
   * Ctor.
   *
   * Constructs from two ranges of the same length with convertible values.
   *
   * @tparam R1 Range with convertible value type
   * @tparam R2 Range with convertible value type
   *
   * @param xs Knot points
   * @param ys Knot values
   */
  template <typename R1, typename R2>
  natural_spline(
    R1&& xs,
    R2&& ys,
    range_ctor_tag = {},
    constraint_t<valid_range<R1> && valid_range<R2>> = 0)
  {
    // number of knot points (>=2 required) + check
    // TODO: SFINAE constraints for size() member or use std::distance()
    auto n = std::size(xs);
    if (n < 2u)
      throw std::runtime_error{"at least 2 knot points required"};
    // size check
    if (auto ny = std::size(ys); n != ny)
      throw std::runtime_error{
        "number of knot points " + std::to_string(n) +
        " != number of knot values " + std::to_string(ny)
      };
    // knots must be monotone
    if (!std::is_sorted(std::begin(xs), std::end(xs)))
      throw std::runtime_error{"knot points must be monotone increasing"};
    // allocate + copy to xs_, ys_
    xs_ = decltype(xs_)(n);
    ys_ = decltype(ys_)(n);
    std::copy(std::begin(xs), std::end(xs), xs_.begin());
    std::copy(std::begin(ys), std::end(ys), ys_.begin());
    // allocate + solve for ms_
    solve();
  }

  /**
   * Evaluate the natural cubic spline at the given point.
   *
   * If the point is outside the interpolation interval the corresponding
   * endpoint cubic polynomial will be used for extrapolation.
   *
   * @param v Evaluation point
   */
  auto operator()(T v) const noexcept
  {
    // determine bucket. i will be in {0, ... xs_.size() - 2}
    std::size_t i = 0u;
    while ((i + 2 < xs_.size()) && (v > xs_[i + 1]))
      i++;
    // compute xs_[i + 1] - xs_[i] difference + 6 * difference
    auto dx = xs_[i + 1] - xs_[i];
    auto d6x = 6 * dx;
    // evaluate xs_[i + 1] - v and v - xs_[i] + cubes
    auto dxu = xs_[i + 1] - v;
    auto dxd = v - xs_[i];
    auto dxu3 = dxu * dxu * dxu;
    auto dxd3 = dxd * dxd * dxd;
    // evaluate polynomial
    return (
      (ms_[i] * dxu3 / d6x) + (ms_[i + 1u] * dxd3 / d6x) +
      (ys_[i] / dx - ms_[i] * dx / 6) * dxu +
      (ys_[i + 1u] / dx - ms_[i + 1u] * dx / 6) * dxd
    );
  }

private:
  std::vector<T> xs_;  // knot points
  std::vector<T> ys_;  // knot values
  std::vector<T> ms_;  // knot point second derivatives

  /**
   * Solve for the natural spline knot point second derivatives.
   *
   * This should be called after the knot points and values are initialized and
   * allocates + writes the solved second derivative values to `ms_`.
   */
  void solve()
  {
    // number of elements
    auto n = xs_.size();
    // n - 1 xs differences
    std::vector<T> dxs(n - 1);
    for (std::size_t i = 0u; i < n - 1; i++)
      dxs[i] = xs_[i + 1] - xs_[i];
    // n - 1 ys differences
    std::vector<T> dys(n - 1);
    for (std::size_t i = 0u; i < n - 1; i++)
      dys[i] = ys_[i + 1] - ys_[i];
    // n - 2 xs differences
    std::vector<T> dx2s(n - 2);
    for (std::size_t i = 0u; i < n - 2; i++)
      dx2s[i] = xs_[i + 2] - xs_[i];
    // coefficient vector of divided differences for the linear system RHS
    std::vector<T> rhs(n);
    // rhs[0] required 0 by natural spline
    rhs[0] = 0;
    // rhs[1] through rhs[n - 2] are 6 * [i - 1, i, i + 1] divided differences
    for (std::size_t i = 1u; i < n - 1; i++)
      rhs[i] = 6 * (dys[i] / dxs[i] - dys[i - 1] / dxs[i - 1]) / dx2s[i - 1];
    // rhs[n - 1] required 0 by natural spline
    rhs[n - 1] = 0;
    // tridiagonal matrix main diagonal values
    std::vector<T> ms(n, 2);
    // lower + upper diagonal tridiagonal matrix values
    std::vector<T> ls(n - 1);
    std::vector<T> us(n - 1);
    // us[0] required 0 by natural spline
    us[0] = 0;
    // fill ls[0], us[1] through ls[n - 2], us[n - 1]
    for (std::size_t i = 1u; i < n - 1; i++) {
      auto u = dxs[i - 1] / dx2s[i - 1];
      ls[i - 1] = u;
      us[i] = 1 - u;
    }
    // ls[n - 2] required 0 by natural spline
    ls[n - 2] = 0;
    // solve tridiagonal system
    ms_ = decltype(ms_)(n);
// MSVC emits C4868 for tridiagonal_view<T> ctor taking span<T> by copy
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4868)
    tridiagonal_view tri{ls, ms, us};
PDMATH_MSVC_WARNINGS_POP()
    tri(ms_, rhs);
  }
};

/**
 * User-defined deduction guide for the `natural_spline<T>`.
 *
 * This deduces to the wider of the two range value types.
 *
 * @tparam R1 Range of floating values
 * @tparam R2 Range of floating values
 */
template <typename R1, typename R2>
natural_spline(R1&&, R2&&) -> natural_spline<
  std::common_type_t<range_value_t<R1>, range_value_t<R2>>
>;

/**
 * User-defined deduction guide for the `natural_spline<T>`.
 *
 * This deduces to the wider of the two initializer list value types and is
 * provided separately due to template deduction rules.
 *
 * @tparam T1 Floating-point type
 * @tparam T2 Floating-point type
 */
template <typename T1, typename T2>
natural_spline(
  std::initializer_list<T1>,
  std::initializer_list<T2>) -> natural_spline<std::common_type_t<T1, T2>>;

}  // namespace pdmath

#endif  // PDMATH_NATURAL_SPLINE_H_
