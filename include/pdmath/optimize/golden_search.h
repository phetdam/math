/**
 * @file golden_search.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Boost-like golden-section search function
 * @copyright MIT License
 */

#ifndef PDMATH_OPTIMIZE_GOLDEN_SEARCH_H_
#define PDMATH_OPTIMIZE_GOLDEN_SEARCH_H_

#include <cstdint>

#include "pdmath/optimize/functor_base.h"
#include "pdmath/optimize/optimize_result.h"

namespace pdmath {
namespace optimize {

/**
 * Golden-section search for a scalar function.
 * 
 * @param f `F` callable to find a root of
 */
template <class F, class T>
scalar_optimize_result<T> golden_search(
  F f,
  T guess,
  T lbound,
  T ubound,
  double tol)
{
  std::uintmax_t n_iter = 0;
  std::uintmax_t n_fev = 0;
  // dummy return
  return scalar_optimize_result<T>(
    T(0),
    true,
    "Definitely converged lmao",
    100,
    f(T(0))
  );
}

} // namespace pdmath::optimize
} // namespace pdmath

#endif // PDMATH_OPTIMIZE_GOLDEN_SEARCH_H_
