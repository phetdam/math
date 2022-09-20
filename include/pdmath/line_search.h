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

#include <Eigen/Core>

#include "pdmath/bases.h"
#include "pdmath/helpers.h"
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
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t>
class direction_search : public fev_mixin, public gev_mixin, public hev_mixin {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);
  virtual ~direction_search() = default;
  virtual V_t operator()(const V_t&) = 0;
};

/**
 * Search direction computation functor that uses the negative gradient.
 *
 * @tparam G Gradient callable taking a `const V_t&` returning `V_t`
 * @tparam V_t *Container* type representing a vector
 */
template <typename G, typename V_t = Eigen::VectorXd>
class steepest_direction_search : public direction_search<V_t> {
public:
  using gradient_type = G;
  PDMATH_USING_CONTAINER_TYPES(V_t);

  /**
   * `steepest_direction_search` constructor.
   *
   * @param grad `G` gradient callable copied as a member.
   */
  steepest_direction_search(G grad) : grad_(std::move(grad)) {}

  steepest_direction_search() = delete;

  /**
   * Compute a search direction given the previous solution guess.
   *
   * @param x `const V_t&` previous solution guess
   */
  V_t operator()(const V_t& x) override
  {
    V_t dir = grad_(x);
    this->n_gev_++;
    // allow more than just "vector" classes with operator- overloads
    std::transform(
      dir.cbegin(), dir.cend(), dir.begin(), std::negate<element_type>()
    );
    return dir;
  }

private:
  G grad_;
};

/**
 * Base template class for search direction convergence policy functors.
 *
 * When called on the search direction, returns `true` to signal convergence.
 *
 * @note For each invocation of `operator()`, the number of function, gradient,
 *     and/or Hessian evaluations should be updated as necessary.
 *
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t>
class direction_policy : public fev_mixin, public gev_mixin, public hev_mixin {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);
  virtual ~direction_policy() = default;
  virtual bool operator()(const V_t&) = 0;
};

/**
 * Search direction policy that always returns `false` on invocation.
 *
 * Useful when one only wants the maximum number of iterations to determine
 * when the `line_search` method or its wrappers should terminate.
 *
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t>
class no_direction_policy : public direction_policy<V_t> {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);
  bool operator()(const V_t& /*dir*/) override { return false; }
};

/**
 * Search direction policy where direction below a minimum implies convergence.
 *
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t>
class min_norm_direction_policy : public direction_policy<V_t> {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);

  /**
   * Constructor.
   *
   * @param min `element_type` minimum search direction norm that must be
   *     exceeded during the routine's execution to prevent early convergence.
   * @param norm `const norm<T, V_t>&` norm functor to compute the norm. The
   *     default is a default-constructed `p_norm`, i.e. the 2-norm.
   */
  min_norm_direction_policy(
    element_type min = 1e-6, const norm<V_t>& norm = p_norm())
    : min_norm_(min), norm_(norm)
  {}

  /**
   * Return `true` if the norm of `d` is <= minimum norm.
   *
   * @param dir `const V_t&` search direction vector
   */
  bool operator()(const V_t& dir) override
  {
    return norm_(dir) <= min_norm_ ? true : false;
  }

  /**
   * Return minimum norm that must be exceeded to prevent early convergence.
   */
  const element_type& min_norm() const { return min_norm_; }

  /**
   * Return norm functor used to compute gradient norms.
   */
  const pdmath::norm<V_t>& norm() const { return norm_; }

private:
  element_type min_norm_;
  pdmath::norm<V_t> norm_;
};

/**
 * Template base class for a step search functor.
 *
 * Should be subclassed when implementing one's own step search method.
 *
 * @note For each invocation of `operator()`, the number of function, gradient,
 *     and/or Hessian evaluations should be updated as necessary.
 *
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t>
class step_search : public fev_mixin, public gev_mixin, public hev_mixin {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);

  /**
   * Compute a step size from the previous guess and the new search direction.
   *
   * @param x_p `const V_t&` previous solution guess
   * @param dir `const V_t&` new search direction to update `x_p` with
   */
  virtual element_type operator()(const V_t& x_p, const V_t& dir) = 0;

  /**
   * Return the last computed step size.
   */
  virtual const element_type& last_step() const = 0;
};

/**
 * Constant step length line search implementation.
 *
 * Use to represent a line search strategy that only uses a fixed step size.
 *
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t = Eigen::VectorXd>
class const_step_search : public step_search<V_t> {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);

  /**
   * `const_step_search` constructor.
   *
   * @param eta `element_type` positive step size to use
   */
  const_step_search(element_type eta = 0.1)
  {
    assert(eta > 0);
    eta_ = eta;
  }

  /**
   * Return the step size, which is always `eta_`.
   *
   * @param x_p `const V_t&` previous solution guess
   * @param dir `const V_t&` new search direction to update `x_p` with
   */
  element_type operator()(const V_t& /* x_p */, const V_t& /* dir */) override
  {
    return eta_;
  }

  /**
   * Return the last computed step length, which is always `eta_`.
   */
  const element_type& last_step() const override { return eta_; }

private:
  element_type eta_;
};

/**
 * Backtracking line search implementation with constant contraction factor.
 *
 * Implementation from Nocedal and Wright's *Numerical Optimization*, page 37.
 * Defaults for `c1`, the Armijo condition damping factor, and `rho`, the
 * step size shrinkage factor, from Hastie, Tibshirani, and Wainwright's
 * monograph *Statistical Learning with Sparsity*, page 102.
 *
 * @tparam F_o objective, must take a `const V_t&` and return `V_t::value_type`
 * @tparam F_g gradient, must take a `const V_t&` and return `V_t::value_type`
 * @tparam V_t *Container* type representing a vector
 */
template <typename F_o, typename F_g, typename V_t = Eigen::VectorXd>
class backtrack_step_search : public step_search<V_t> {
public:
  using objective_type = F_o;
  using gradient_type = F_g;
  PDMATH_USING_CONTAINER_TYPES(V_t);

  /**
   * Constructor.
   *
   * @param func `F_o` objective, takes `const V_t&`, returns `element_type`
   * @param grad `F_g` gradient, takes `const V_t&`, returns `element_type`
   * @param eta0 `T` positive starting step size to use
   * @param c1 `element_type` Armijo condition damping factor in (0, 1)
   *     affecting the sufficient decrease condition. Nocedal and Wright
   *     recommend `1e-4`; Hastie, Tibshirani, and Wainwright recommend `0.5`.
   * @param rho `element_type` step size shrinkage factor in (0, 1). Hastie,
   *     Tibshirani, and Wainwright recommend `0.8` as a choice.
   */
  backtrack_step_search(
    F_o func,
    F_g grad,
    element_type eta0,
    element_type c1 = 0.5,
    element_type rho = 0.8)
    : func_(func), grad_(grad), last_step_(), step_search<V_t>()
  {
    assert(eta0 > 0);
    assert(c1 > 0 && c1 < 1);
    assert(rho > 0 && rho < 1);
    eta0_ = eta0;
    c1_ = c1;
    rho_ = rho;
  }

  backtrack_step_search() = delete;

  /**
   * Compute a step size from the previous guess and the new search direction.
   *
   * Updates number of function + gradient evaluations and last step each time.
   *
   * @param x_p `const V_t&` previous solution guess
   * @param dir `const V_t&` new search direction to update `x_p` with
   */
  element_type operator()(const V_t& x_p, const V_t& dir) override
  {
    // set initial step size, reset function eval count
    element_type eta = eta0_;
    // set initial step size, precompute current objective and gradient value,
    // get inner product of gradient w/ search direction
    element_type f_x = func_(x_p);
    element_type g_x = grad_(x_p);
    element_type ip_x = std::inner_product(
      g_x.cbegin(), g_x.cend(), dir.cbegin(), 0
    );
    // get new direction we use to compute the new function value
    V_t x_c = x_p;
    std::transform(
      x_c.cbegin(),
      x_c.cend(),
      dir.cbegin(),
      x_c.begin(),
      [&](const element_type& a, const element_type& b) { return a + eta * b; }
    );
    // update number of function + gradient evals. note that since this is a
    // template class, we have to use this, as otherwise these members will
    // not be looked up (nondependent name). see the ISO C++ FAQ:
    // https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members
    this->n_fev_ += 1;
    this->n_gev_ += 1;
    // until sufficient decrease is met, keep shrinking
    while (this->n_fev_ += 1, func_(x_c) <= f_x + c1_ * eta * ip_x) {
      eta *= rho_;
      std::transform(
        x_p.cbegin(),
        x_p.cend(),
        dir.cbegin(),
        x_c.begin(),
        [&](const element_type& a, const element_type& b) { return a + eta * b; }
      );
    }
    last_step_ = eta;
    return eta;
  }

  /**
   * Return the last computed step length.
   *
   * On initialization, this returns `element_type()`.
   */
  const element_type& last_step() const override { return last_step_; }

private:
  F_o func_;
  F_g grad_;
  element_type last_step_;
  element_type eta0_;
  element_type c1_;
  element_type rho_;
};

/**
 * Templated [accelerated] line search descent implementation for `std` types.
 *
 * @note `func`, `dir_search`, `eta_search`, `dir_policy` are all passed
 *     *by value*, as the base classes for these functor types all them to
 *     implement a non-`const` `operator()`.
 *
 * @tparam V_t *Container* type representing a vector
 * @tparam M_t matrix type with `typename V_t::value_type` elements
 * @tparam F_t callable for per-iteration update of guess post-update with the
 *     scaled search direction, ex. proximal operator such as the
 *     soft-thresholding operator, or a  projection operator. Must take a
 *     `const V_t&` and then return a `V_t`.
 *
 * @param func `func_functor<V_t, typename V_t::value_type, V_t, M_t>` functor
 *     giving the objective function, optionally with gradient and Hessian.
 * @param dir_search `direction_search<V_t>` search direction functor, which
 *     when evaluated takes the `const V_t&` current guess and returns the
 *     `V_t` search direction to update along. The returned search direction
 *     need not be a descent direction in general.
 * @param eta_search `step_search<T, V_t>` step line search functor, which
 *     when evaluated takes the `const V_t&` current guess and search direction
 *     and returns the `typename V_t::value_type` step size to use.
 * @param x0 `const V_t&` initial guess for the line search
 * @param max_iter `std::uintmax_t` max iterations allowed
 * @param dir_policy `direction_policy<V_t>&` convergence policy based off of
 *     the search direction, which when evaluated takes the `const V_t&` search
 *     direction. Returns `true` to indicate convergence.
 * @param nesterov `bool` flag, `true` to use Nesterov's momentum scheme
 * @param tail_transform `F_t` callable for per-iteration transform of updated
 *     guess after updating previous guess with scaled search direction
 */
template <typename V_t, typename M_t, typename F_t>
optimize_result<typename V_t::value_type, V_t, V_t, M_t> line_search(
  func_functor<V_t, typename V_t::value_type, V_t, M_t> func,
  direction_search<V_t> dir_search,
  step_search<V_t> eta_search,
  const V_t& x0,
  std::uintmax_t max_iter,
  direction_policy<V_t> dir_policy,
  bool nesterov = false,
  F_t tail_transform = identity_functor<V_t>())
{
  using T = typename V_t::value_type;
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
      x_c.cbegin(),
      x_c.cend(),
      dx.cbegin(),
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
        x_c.cbegin(),
        x_c.cend(),
        x_p.cbegin(),
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
  return optimize_result<T, V_t, V_t, M_t>(
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
