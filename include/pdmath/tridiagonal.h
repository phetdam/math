/**
 * @file tridiagonal.h
 * @author Derek Huang
 * @brief C++ header for solving tridiagonal matrix systems
 * @copyright MIT License
 */

#ifndef PDMATH_TRIDIAGONAL_H_
#define PDMATH_TRIDIAGONAL_H_

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "pdmath/features.h"
#include "pdmath/span.h"
#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * Sparse view representing a tridiagonal matrix.
 *
 * This contains only 3 spans, one for each relevant diagonal, and can be used
 * in matrix-like operations as a space-efficient view.
 *
 * Attempting to set any elements not on the three diagonals is UB.
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
  T d_{};          // default-initialized value
};

/**
 * User-defined deduction guide.
 *
 * This ensures that the value type can be deduced from the input ranges.
 *
 * @tparam R1 First range type
 * @tparam R2 Second range type
 * @tparam R3 Third range type
 */
template <typename R1, typename R2, typename R3>
tridiagonal_view(R1&&, R2&&, R3&&) ->
tridiagonal_view<std::remove_reference_t<range_reference_t<R1>>>;

/**
 * Write the `tridiagonal_view<T>` to the output stream.
 *
 * This outputs the `tridiagonal_view<T>` in dense form for human display. For
 * example, given a lower diagonal 0.5, 0.5, 0.5, 0.6, a main diagonal of all
 * 2s, and an upper diagonal 0.5, 0.6, 0.6, 0.4, the output would be:
 *
 * @code
 *   2 0.5   0   0   0
 * 0.5   2 0.6   0   0
 *   0 0.4   2 0.6   0
 *   0   0 0.4   2 0.4
 *   0   0   0 0.6   2
 * @endcode
 *
 * @tparam T Element type
 *
 * @param out Output stream
 * @param tri Tridiagonal matrix view
 */
template <typename T>
auto& operator<<(std::ostream& out, const tridiagonal_view<T>& tri)
{
  // lower, main, upper diagonal views
  auto ld = tri.ld();
  auto md = tri.md();
  auto ud = tri.ud();
  // string representations of lower, main, upper diagonal elements
  std::vector<std::string> ls(ld.size());
  std::vector<std::string> ms(md.size());
  std::vector<std::string> us(ud.size());
  // column column display widths
  std::vector<std::size_t> col_widths(tri.cols());
  // string representation helper. this is necessary since std::to_string()
  // uses fixed decimal representations which can be overly verbose
  auto to_string = [](auto&& v)
  {
    std::stringstream ss;
    ss << v;
// note: in C++20 we can use the rvalue ref-qualified str() to avoid a copy
#if PDMATH_HAS_CXX20
    return std::move(ss).str();
#else
    return ss.str();
#endif  // !PDMATH_HAS_CXX20
  };
  // compute string representations for diagonal elements
  for (std::size_t i = 0u; i < ls.size(); i++)
    ls[i] = to_string(ld[i]);
  for (std::size_t i = 0u; i < ms.size(); i++)
    ms[i] = to_string(md[i]);
  for (std::size_t i = 0u; i < us.size(); i++)
    us[i] = to_string(ud[i]);
  // compute column widths
  col_widths.front() = (std::max)(ls.front().size(), ms.front().size());
  for (std::size_t i = 1u; i < tri.cols() - 1u; i++)
    col_widths[i] = (std::max)({ls[i].size(), ms[i].size(), us[i].size()});
  col_widths.back() = (std::max)(ms.back().size(), us.back().size());
  // padding helper
  auto pad = [&out](std::size_t n)
  {
    for (std::size_t i = 0u; i < n; i++)
      out.put(' ');
  };
  // write values right-justified with appropriate padding
  for (std::size_t i = 0u; i < tri.rows(); i++) {
    // start new line
    if (i)
      out.put('\n');
    // row values
    for (std::size_t j = 0u; j < tri.cols(); j++) {
      // extra space to separate columns
      if (j)
        pad(1);
      // lower diagonal
      if (i == j + 1) {
        pad(col_widths[j] - ls[j].size());
        out << ls[j];
      }
      // main diagonal
      else if (i == j) {
        pad(col_widths[j] - ms[j].size());
        out << ms[j];
      }
      // upper diagonal
      else if (i + 1 == j) {
        pad(col_widths[j] - us[i].size());
        out << us[i];
      }
      // write a zero
      else {
        pad(col_widths[j] - 1u);
        out << 0.;
      }
    }
  }
  // done
  return out;
}

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
  for (std::size_t i = 1u; i < n - 1; i++)
    cs[i] /= (md[i] - ld[i - 1] * cs[i - 1]);
  // iterate RHS coefficients
  ds[0] /= md[0];
  for (std::size_t i = 1u; i < n; i++)
    ds[i] = (ds[i] - ld[i - 1] * ds[i - 1]) / (md[i] - ld[i - 1] * cs[i - 1]);
  // back-substitute result
  res[n - 1] = ds[n - 1];
  for (std::size_t i = 0u; i < n - 1; i++)
    res[n - i - 2] = ds[n - i - 2] - cs[n - i - 2] * res[n - i - 1];
}

}  // namespace pdmath

#endif  // PDMATH_TRIDIAGONAL_H_
