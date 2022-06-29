/**
 * @file line_search.h
 * @author Derek Huang
 * @brief C++ header for [accelerated] line search implementation
 * @copyright MIT License
 */

#ifndef PDMATH_LINE_SEARCH_H_
#define PDMATH_LINE_SEARCH_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <utility>

#include "pdmath/bases.h"
#include "pdmath/mixins.h"
#include "pdmath/norms.h"
#include "pdmath/optimize_result.h"
#include "pdmath/types.h"

namespace pdmath {

/**
 * Base template class for search direction computation functors.
 *
 * When called on the previous solution guess, returns a search direction to
 * be scaled by a step size for updating the solution guess.
 *
 * @note For each invocation of `operator()`, the number of function, gradient,
 *     and/or Hessian evaluations should be updated as necessary.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 */
template <class T = double, class V_t = boost_vector<T>>
class direction_search
  : public fev_reset_mixin, public gev_reset_mixin, public hev_reset_mixin {
public:
  virtual ~direction_search() = default;
  virtual V_t operator()(const V_t&) = 0;
};

/**
 * Base template class for search direction convergence policy functors.
 *
 * When called on the search direction, returns `true` to signal convergence.
 *
 * @note For each invocation of `operator()`, the number of function, gradient,
 *     and/or Hessian evaluations should be updated as necessary.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 */
template <class T = double, class V_t = boost_vector<T>>
class direction_policy
  : public fev_reset_mixin, public gev_reset_mixin, public hev_reset_mixin {
public:
  virtual ~direction_policy() = default;
  virtual bool operator()(const V_t&) = 0;
};

/**
 * Search direction policy where direction below a minimum implies convergence.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 */
template <class T = double, class V_t = boost_vector<T>>
class min_norm_direction_policy : public direction_policy<T, V_t> {
public:
  /**
   * Constructor.
   *
   * @param min_norm `T` minimum search direction norm that must be exceeded
   *     during the routine's execution to prevent early convergence.
   * @param norm `const norm<T, V_t>&` norm functor to compute the norm. The
   *     default is a default-constructed `p_norm`, i.e. the 2-norm.
   */
  min_norm_direction_policy(T min_norm, const norm<T, V_t>& norm = p_norm())
    : min_norm_(min_norm), norm_(norm)
  {}

  /**
   * Return `true` if the norm of `d` is <= minimium norm.
   *
   * @param dir `const V_t&` search direction vector
   */
  bool operator()(const V_t& dir)
  {
    return norm_(dir) <= min_norm_ ? true : false;
  }

  /**
   * Return minimum norm that must be exceeded to prevent early convergence.
   */
  const T& min_norm() const { return min_norm_; }

  /**
   * Return norm functor used to compute gradient norms.
   */
  const norm<T, V_t> norm() const { return norm_; }

private:
  T min_norm_;
  norm<T, V_t> norm_;
};

/**
 * Template base class for a step search functor.
 *
 * Should be subclassed when implementing one's own step search method.
 *
 * @note For each invocation of `operator()`, the number of function, gradient,
 *     and/or Hessian evaluations should be updated as necessary.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 */
template <class T = double, class V_t = boost_vector<T>>
class step_search
  : public fev_reset_mixin, public gev_reset_mixin, public hev_reset_mixin {
public:
  /**
   * Compute a step size from the previous guess and the new search direction.
   *
   * @param x_p `const V_t&` previous solution guess
   * @param dir `const V_t&` new search direction to update `x_p` with
   */
  virtual T operator()(const V_t& x_p, const V_t& dir) = 0;
};

/**
 * Backtracking line search implementation with constant contraction factor.
 *
 * Implementation from Nocedal and Wright's *Numerical Optimization*, page 37.
 * Defaults for `c1`, the Armijo condition damping factor, and `rho`, the
 * step size shrinkage factor, from Hastie, Tibshirani, and Wainwright's
 * monograph *Statistical Learning with Sparsity*, page 102.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 * @tparam F_o objective, must take a `const V_t&` and return `T`
 * @tparam F_g gradient, must take a `const V_t&` and return `V_t`
 */
template <
  class T = double,
  class V_t = boost_vector<T>,
  class F_o = functor_base<V_t, T>,
  class F_g = functor_base<V_t>
>
class backtrack_step_search : public step_search {
public:
  /**
   * Constructor.
   *
   * @param func `F_o` objective, which takes a `const V_t&` and returns a `T`
   * @param grad `F_g` gradient, which takes a `const V_t&` and returns `V_t`
   * @param eta0 `T` positive starting step size to use
   * @param c1 `T` Armijo condition damping factor in (0, 1) affecting the
   *     sufficient decrease condition. Nocedal and Wright recommand `1e-4` as
   *     a choice, while Hastie, Tibshirani, and Wainwright recommend `0.5`.
   * @param rho `T` step size shrinkage factor in (0, 1). Hastie, Tibshirani,
   *     and Wainwright recommend `0.8` as a choice.
   */
  backtrack_step_search(F_o func, F_g grad, T eta0, T c1 = 0.5, T rho = 0.8)
    : func_(func), grad_(grad)
  {
    assert(eta0 > 0);
    assert(c1 > 0 && c1 < 1);
    assert(rho > 0 && rho < 1);
    eta0_ = eta0;
    c1_ = c1;
    rho_ = rho;
  }

  /**
   * Compute a step size from the previous guess and the new search direction.
   *
   * Updates the number of function and gradient evaluations each time.
   *
   * @param x_p `const V_t&` previous solution guess
   * @param dir `const V_t&` new search direction to update `x_p` with
   */
  T operator()(const V_t& x_p, const V_t& dir)
  {
    // set initial step size, reset function eval count
    T eta = eta0_;
    // set initial step size, precompute current objective and gradient value,
    // get inner product of gradient w/ search direction
    T f_x = func_(x_p);
    T g_x = grad_(x_p);
    T ip_x = std::inner_product(g_x.begin(), g_x.end(), dir.begin(), 0);
    // get new direction we use to compute the new function value
    V_t x_c = x_p;
    std::transform(
      x_c.begin(), x_c.end(), dir.begin(), x_c.begin(), std::plus<T>()
    );
    // update number of function + gradient evals
    n_fev_ += 1;
    n_gev_ += 1;
    // until sufficient decrease is met, keep shrinking
    while (n_fev_ += 1, func_(x_c) <= f_x + c1_ * eta * ip_x) {
      eta *= rho_;
      std::transform(
        x_p.begin(), x_p.end(), dir.begin(), x_c.begin(), std::plus<T>()
      );
    }
    return eta;
  }

private:
  F_o func_;
  F_g grad_;
  T eta0_;
  T c1_;
  T rho_;
};

/**
 * Templated [accelerated] line search descent implementation for `std` types.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 * @tparam M_t matrix type, with `T` elements
 * @tparam F_t callable for per-iteration update of guess post-update with the
 *     scaled search direction, ex. proximal operator such as the
 *     soft-thresholding operator, or a  projection operator. Must take a
 *     `const V_t&` and then return a `V_t`.
 *
 * @param func `const function_functor<V_t, T, V_t, M_t>&` functor giving
 *     the objective function, optionally with gradient and Hessian.
 * @param dir_search `direction_search<T, V_t>&` search direction functor,
 *     which when evaluated takes the `const V_t&` current guess and returns
 *     the `V_t` search direction to update along. The returned search
 *     direction need not be a descent direction.
 * @param eta_search `step_search<T, V_t>&` step line search functor, which
 *     when evaluated takes the `const V_t&` current guess and search direction
 *     and returns the `T` step size to use.
 * @param x0 `const V_t&` initial guess for the line search
 * @param max_iter `std::uintmax_t` max iterations allowed
 * @param dir_policy `direction_policy<T, V_t>&` convergence policy based off
 *     of the search direction, which when evaluated takes the `const V_t&`
 *     search direction. Returns `true` to indicate convergence.
 * @param nesterov `bool` flag, `true` to use Nesterov's momentum scheme
 * @param tail_transform `F_t` callable for per-iteration transform of updated
 *     guess after updating previous guess with scaled search direction
 */
template <
  class T = double,
  class V_t = boost_vector<T>,
  class M_t = boost_matrix<T>,
  class F_t = functor_base<V_t>
>
vector_optimize_result<T> line_search(
  const function_functor<V_t, T, V_t, M_t>& func,
  direction_search<T, V_t>& dir_search,
  step_search<T, V_t>& eta_search,
  const V_t& x0,
  std::uintmax_t max_iter,
  direction_policy<T, V_t>& dir_policy,
  bool nesterov = false,
  F_t tail_transform = identity_functor<V_t>)
{
  bool converged = false;
  // container for the current and previous solution guesses. we only need the
  // previous guesses in the case of Nesterov acceleration
  V_t x_p = x0;
  V_t x_c = x0;
  // container for Nesterov "lookahead" guesses
  V_t z = x0;
  // new search direction, step size chosen by line search, iteration count
  V_t dx;
  T eta;
  std::uintmax_t n_iter = 0;
  // keep going as long as iteration count isn't reached and evaluated descent
  // direction does not meet the direction convergence policy
  while (
    n_iter < max_iter &&
    !(converged = dir_policy(dx = dir_search((nesterov) ? z : x_c)))
  ) {
    eta = eta_search((nesterov) ? z : x_c, dx);
    // update the current guess and perform final transform
    std::transform(
      x_c.begin(),
      x_c.end(),
      dx.begin(),
      x_c.begin(),
      [&](T x_c_v, T dx_v) { return x_c_v + eta * dx_v; }
    );
    // for (std::size_t i = 0; i < x_c.size(); i++) {
    //     x_c[i] += eta * dx[i];
    // }
    x_c = tail_transform(x_c);
    // need to update the "lookahead" sequence if using acceleration and also
    // update the previous guess to match the current guess
    if (nesterov) {
      std::transform(
        x_c.begin(),
        x_c.end(),
        x_p.begin(),
        z.begin(),
        [&](T x_c_v, T x_p_v)
        {
          return x_c_v + (n_iter + 1) / (n_iter + 4) * (x_c_v - x_p_v);
        }
      );
      // for (std::size_t i = 0; i < x_c.size(); i++) {
      //   z[i] = x_c[i] + (n_iter + 1) / (n_iter + 4) * (x_c[i] - x_p[i]);
      // }
      x_p = x_c;
    }
    n_iter++;
  }
  return vector_optimize_result<T, V_t>(
    x_c,
    converged,
    (converged) ? "Converged by direction policy" : "Iteration limit reached",
    n_iter,
    func.f(x_c),
    1 + dir_search.n_fev() + eta_search.n_fev() + dir_policy.n_fev(),
    func.d1(x_c),
    1 + dir_search.n_fev() + eta_search.n_gev() + dir_policy.n_gev(),
    func.d2(x_c),
    1 + dir_search.n_hev() + eta_search.n_hev() + dir_policy.n_hev()
  );
}

}  // namespace pdmath

#endif  // PDMATH_LINE_SEARCH_H_