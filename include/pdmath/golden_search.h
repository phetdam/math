/**
 * @file golden_search.h
 * @author Derek Huang
 * @brief Boost-like golden-section search function for standard types
 * @copyright MIT License
 */

#ifndef PDMATH_GOLDEN_SEARCH_H_
#define PDMATH_GOLDEN_SEARCH_H_

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "pdmath/optimize_result.h"
#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * Templated golden-section search for a scalar function's minimum.
 *
 * Some aspects are borrowed from Numerical Recipes in C. Implementation is
 * mostly an interpretation of the golden-section search described in Xin Li's
 * notes for Lecture 15 of CMU's 18-660 optimization course.
 *
 * @tparam F Unary callable
 * @tparam T Floating-point type
 *
 * @param f Univariate callable to find a minimum of
 * @param lbound Lower endpoint of search interval
 * @param ubound Uper endpoint of search interval
 * @param tol Termination tolerance
 */
template <typename T, typename F>
optimize_result<T> golden_search(
  F&& f,
  T lbound,
  T ubound,
  T tol = std::sqrt(std::numeric_limits<T>::epsilon()),
  constraint_t<std::is_floating_point_v<T> && std::is_invocable_r_v<T, F, T>> = 0)
{
  assert(lbound < ubound);
  std::uintmax_t n_iter = 0;
  std::uintmax_t n_fev = 0;
  // larger fraction of bracketing interval that is golden ratio times the
  // smaller fraction of the bracketing interval (hence the name). this is
  // usually called 'w' and is also from Numerical Recipes in C.
  T w(1.5 - std::sqrt(5.) / 2.);
  // lower and upper guesses, lbound < lguess < uguess < ubound, and shift
  T gshift = w * (ubound - lbound);
  T lguess(lbound + gshift);
  T uguess(ubound - gshift);
  // values of f at lbound, ubound, lguess, uguess
  T flbound(f(lbound));
  T fubound(f(ubound));
  T flguess(f(lguess));
  T fuguess(f(uguess));
  // safety check: lguess, uguess must yield a lower value than lbound, ubound
  assert(
    (flguess < flbound) ||
    (flguess < fubound) ||
    (fuguess < flbound) ||
    (fuguess < fubound)
  );
  n_fev += 4;
  // termination condition from Numerical Recipes in C
  while (
    std::abs(ubound - lbound) >
    tol * (std::abs(lguess) + std::abs(uguess))
  ) {
    // choose minimum bracketing based on flguess, fuguess, shifting bounds
    if (flguess < fuguess) {
      ubound = uguess;
      fubound = fuguess;
    }
    else {
      lbound = lguess;
      flbound = flguess;
    }
    // re-compute the shift, lower + upper guesses and their function values
    gshift = w * (ubound - lbound);
    lguess = lbound + gshift;
    uguess = ubound - gshift;
    flguess = f(lguess);
    fuguess = f(uguess);
    n_iter++;
    n_fev += 2;
  }
  // choose midpoint of lbound, ubound as result. note we multiply individually
  // to prevent overflow or rounding error from subtraction.
  T res(0.5 * lbound + 0.5 * ubound);
  return {
    res,
    true,
    "Converged within tolerance",
    n_iter,
    f(res),
    // +1 to n_fev since we evaluate f again at res above
    1 + n_fev
  };
}

}  // namespace pdmath

#endif  // PDMATH_GOLDEN_SEARCH_H_
