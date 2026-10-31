/**
 * @file tridiagonal.h
 * @author Derek Huang
 * @brief C++ header for solving tridiagonal matrix systems
 * @copyright MIT License
 */

#ifndef PDMATH_TRIDIAGONAL_H_
#define PDMATH_TRIDIAGONAL_H_

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "pdmath/span.h"
#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * Sparse view representing a tridiagonal matrix.
 *
 * This contains only 3 spans, one for each relevant diagonal, and can be used
 * in matrix-like operations as a space-efficient view.
 *
 * Attempting to set any elements not on the three diagonals is an error.
 *
 * @tparam T Element type
 */
template <typename T>
class tridiagonal_view {
public:
  /**
   * Default ctor.
   *
   * This is provided for convenience but the view is of little utility.
   */
  constexpr tridiagonal_view() noexcept = default;

  /**
   * Ctor.
   *
   * @param ld Lower diagonal of n - 1 elements
   * @param md Main diagonal of n elements
   * @param ud Upper diagonal of n - 1 elements
   */
  constexpr tridiagonal_view(span<T> ld, span<T> md, span<T> ud)
  {
    // size checks
    if (ld.size() != ud.size())
      throw std::runtime_error{
        "lower diagonal size " + std::to_string(ld.size()) +
        " != upper diagonal size " + std::to_string(ud.size())
      };
    if (ld.size() + 1 != md.size())
      throw std::runtime_error{
        "lower diagonal size " + std::to_string(ld.size()) +
        " is not 1 less than main diagonal size " + std::to_string(md.size())
      };
    // initialize
    ld_ = ld;
    md_ = md;
    ud_ = ud;
  }

  /**
   * Return a view of the lower diagonal.
   */
  constexpr auto& ld() const noexcept { return ld_; }

  /**
   * Return a view of the main diagonal.
   */
  constexpr auto& md() const noexcept { return md_; }

  /**
   * Return a view of the upper diagonal.
   */
  constexpr auto& ud() const noexcept { return ud_; }

  /**
   * Return the number of rows in the corresponding tridiagonal matrix.
   */
  constexpr auto rows() const noexcept
  {
    return md_.size();
  }

  /**
   * Return the number of columns in the corresponding tridiagonal matrix.
   */
  constexpr auto cols() const noexcept
  {
    return md_.size();
  }

  /**
   * Return a reference to the specified element.
   *
   * @param i Row index
   * @param j Column index
   */
  constexpr auto& operator()(std::size_t i, std::size_t j) const noexcept
  {
    // lower diagonal
    if (i == j + 1)
      return ld_[j];
    // main diagonal
    else if (i == j)
      return md_[i];
    // upper diagonal
    else if (i + 1 == j)
      return ud_[i];
    // other elements
    // note: const_cast<T> required to preserve value category
    else
      return const_cast<T&>(d_);
  }

  /**
   * Return a reference to the specified element in the lower diagonal.
   *
   * @param i Element index
   */
  constexpr auto& ld(std::size_t i) const noexcept
  {
    return ld_[i];
  }

  /**
   * Return a reference to the specified element in the main diagonal.
   *
   * @param i Element index
   */
  constexpr auto& md(std::size_t i) const noexcept
  {
    return md_[i];
  }

  /**
   * Return a reference to the specified element in the upper diagonal.
   *
   * @param i Element index
   */
  constexpr auto& ud(std::size_t i) const noexcept
  {
    return ud_[i];
  }

private:
  span<T> ld_;     // lower diagonal
  span<T> md_;     // main diagonal
  span<T> ud_;     // upper diagonal
  mutable T d_{};  // default-initialized value
};

/**
 * Find the solution to the tridiagonal system of linear equations.
 *
 * This provides a low-level interface to mixed-precision solution of the
 * sparse tridiagonal system using the recurrence relation for coefficients.
 * Applications of this function include solving for cubic spline knot point
 * second derivatives or systems of PDE equations with tridiagonal forms.
 *
 * The implementation uses the standard Thomas algorithm for solving.
 *
 * @tparam T Floating-point type
 * @tparam U Floating-point type
 * @tparam V Floating-point type
 */
template <typename T, typename U, typename V>
void solve(
  tridiagonal_view<const T>& lhs,
  span<U> res,
  span<const V> rhs,
  constraint_t<
    std::is_floating_point_v<T> &&
    std::is_floating_point_v<U> &&
    std::is_floating_point_v<V> > = 0)
{
  // dimension checks
  if (res.size() != rhs.size())
    throw std::runtime_error{
      "solution span size " + std::to_string(res.size()) +
      " != RHS span size " + std::to_string(rhs.size())
    };
  if (res.size() != lhs.rows())
    throw std::runtime_error{
      "solution span size " + std::to_string(res.size()) +
      " != LHS dimension " + std::to_string(lhs.rows())
    };
  // record problem dimension + diagonal spans
  auto n = res.size();
  auto ld = lhs.ld();
  auto md = lhs.md();
  auto ud = lhs.ud();
  // upper diagonal iterations + RHS iterations
  std::vector<T> cs{ud.begin(), ud.end()};
  std::vector<V> ds{rhs.begin(), rhs.end()};
  // iterate upper diagonal coefficients
  cs[0] /= md[0];
  for (auto i = 1u; i < n - 1; i++)
    cs[i] /= (md[i] - ld[i - 1] * cs[i - 1]);
  // iterate RHS coefficients
  ds[0] /= md[0];
  for (auto i = 1u; i < n; i++)
    ds[i] = (ds[i] - ld[i - 1] * ds[i - 1]) / (md[i] - ld[i - 1] * cs[i - 1]);
  // back-substitute result
  res[n - 1] = ds[n - 1];
  for (auto i = 0u; i < n - 1; i++)
    res[n - i - 2] = ds[n - i - 2] - cs[n - i - 2] * res[n - i - 1];
}

}  // namespace pdmath

#endif  // PDMATH_TRIDIAGONAL_H_
