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
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <utility>

// boost/math/tools/norms.hpp from Boost 1.74.0 seems to have forgotten to
// indicate which isfinite() is being looked up so we introduce to namespace
namespace boost::math::tools {

using std::isfinite;

}  // namespace boost::math::tools

#include <Eigen/Core>
#include <boost/math/tools/norms.hpp>

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
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t>
class steepest_direction_search : public direction_search<V_t> {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);
  using gradient_function = std::function<V_t(const V_t&)>;

  /**
   * `steepest_direction_search` constructor.
   *
   * @param grad `gradient_function` gradient *Callable* copied as a member.
   */
  steepest_direction_search(gradient_function grad) : grad_(grad) {}

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
  gradient_function grad_;
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
 * Search direction policy where norm below a minimum implies convergence.
 *
 * @tparam N_t `pdmath::norm<V_t>` subclass, `V_t` a *Container*
 */
template <typename N_t>
class min_norm_direction_policy
  : public direction_policy<typename N_t::container_type> {
private:
  using V_t = typename N_t::container_type;

public:
  PDMATH_USING_CONTAINER_TYPES(V_t);
  using norm_type = N_t;
  using norm_pointer_type = std::shared_ptr<N_t>;

  /**
   * Constructor.
   *
   * @param min `element_type` minimum search direction norm that must be
   *     exceeded during the routine's execution to prevent early convergence.
   */
  min_norm_direction_policy(element_type min = 1e-8)
    : min_norm_(min), norm_(new N_t())
  {}

  /**
   * Return `true` if the norm of `d` is <= minimum norm.
   *
   * @param dir `const V_t&` search direction vector
   */
  bool operator()(const V_t& dir) override
  {
    return (*norm_)(dir) <= min_norm_ ? true : false;
  }

  /**
   * Return minimum norm that must be exceeded to prevent early convergence.
   */
  const element_type& min_norm() const { return min_norm_; }

private:
  element_type min_norm_;
  norm_pointer_type norm_;
};

/**
 * Search direction policy where p-norm below a minimum implies convergence.
 *
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t>
using min_p_norm_direction_policy = min_norm_direction_policy<p_norm<V_t>>;

/**
 * Search direction policy where max norm below a minimum implies convergence.
 *
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t>
using min_max_norm_direction_policy = min_norm_direction_policy<max_norm<V_t>>;

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

  virtual ~step_search() = default;

  /**
   * Compute a step size from the previous guess and the new search direction.
   *
   * @param x_p `const V_t&` previous solution guess
   * @param dir `const V_t&` new search direction to update `x_p` with
   */
  virtual element_type operator()(const V_t& x_p, const V_t& dir) = 0;

  /**
   * Return the last computed step size.
   *
   * If `operator()` not yet been invoked, should return `0`.
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
template <typename V_t>
class const_step_search : public step_search<V_t> {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);

  /**
   * `const_step_search` constructor.
   *
   * @param eta `element_type` positive step size to use
   */
  const_step_search(element_type eta = 0.1) : last_step_()
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
    // avoid any conditional comparison and just assign
    last_step_ = eta_;
    return eta_;
  }

  /**
   * Return the last computed step length.
   *
   * If `operator()` has not yet been invoked, returns `0`, else `eta`.
   */
  const element_type& last_step() const override { return last_step_; }

private:
  element_type eta_;
  element_type last_step_;
};

/**
 * Backtracking line search implementation with constant contraction factor.
 *
 * Implementation from Nocedal and Wright's *Numerical Optimization*, page 37.
 * Defaults for `c1`, the Armijo condition damping factor, and `rho`, the
 * step size shrinkage factor, from Hastie, Tibshirani, and Wainwright's
 * monograph *Statistical Learning with Sparsity*, page 102.
 *
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t>
class backtrack_step_search : public step_search<V_t> {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);
  using objective_function = std::function<element_type(const V_t&)>;
  using gradient_function = std::function<V_t(const V_t&)>;

  /**
   * Constructor.
   *
   * @param func `objective_function` objective
   * @param grad `gradient_function` gradient
   * @param eta0 `element_type` positive starting step size to use
   * @param c1 `element_type` Armijo condition damping factor in (0, 1)
   *     affecting the sufficient decrease condition. Nocedal and Wright
   *     recommend `1e-4`; Hastie, Tibshirani, and Wainwright recommend `0.5`.
   * @param rho `element_type` step size shrinkage factor in (0, 1). Hastie,
   *     Tibshirani, and Wainwright recommend `0.8` as a choice.
   */
  backtrack_step_search(
    objective_function func,
    gradient_function grad,
    element_type eta0,
    element_type c1 = 0.5,
    element_type rho = 0.8)
    : func_{std::move(func)}, grad_{std::move(grad)}, last_step_{}
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
   * Updates number of function + gradient evaluations and last step each time.
   *
   * @param x_p `const V_t&` previous solution guess
   * @param dir `const V_t&` new search direction to update `x_p` with
   */
  element_type operator()(const V_t& x_p, const V_t& dir) override
  {
    // set initial step size, reset function eval count
    auto eta = eta0_;
    // set initial step size, precompute current objective and gradient value,
    // get inner product of gradient w/ search direction
    auto f_x = func_(x_p);
    auto g_x = grad_(x_p);
    // MSVC complains that there is potential loss of data converting 0 to
    // float and emits internal conversion warnings, so use ctor explicitly
    auto ip_x = std::inner_product(
      std::begin(g_x), std::end(g_x), std::begin(dir), element_type{0}
    );
    // get new direction we use to compute the new function value
    auto x_c = x_p;
    std::transform(
      std::begin(x_c),
      std::end(x_c),
      std::begin(dir),
      std::begin(x_c),
      // TODO: might want to just make a copy since element_type is arithmetic
      // note: could use eta0_ and just take this pointer by copy
      [eta](const auto& a, const auto& b) { return a + eta * b; }
    );
    // update number of function + gradient evals. note that since this is a
    // template class, we have to use this, as otherwise these members will
    // not be looked up (nondependent name). see the ISO C++ FAQ:
    // https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members
    this->n_fev_ += 1;
    this->n_gev_ += 1;
    // until sufficient decrease is met, keep shrinking
    while (this->n_fev_ += 1, func_(x_c) > f_x + c1_ * eta * ip_x) {
      eta *= rho_;
      std::transform(
        std::begin(x_p),
        std::end(x_p),
        std::begin(dir),
        std::begin(x_c),
        // TODO: see same TODO above
        // note: could be a single named closure that takes eta by refeence
        [eta](const auto& a, const auto& b) { return a + eta * b; }
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

  /**
   * Return the chosen starting step size.
   */
  const element_type& eta0() const { return eta0_; }

  /**
   * Return the Armijo condition damping factor in `(0, 1)` used.
   */
  const element_type& c1() const { return c1_; }

  /**
   * Return the step size shrinking factor in `(0, 1)` used.
   */
  const element_type& rho() const { return rho_; }

private:
  objective_function func_;
  gradient_function grad_;
  element_type last_step_;
  element_type eta0_;
  element_type c1_;
  element_type rho_;
};

// MSVC does not like the C-style assignment-before-evaluation idiom in the
// main while loop. since warning number >= 4700, we disable at function start.
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4706)
#endif  // _MSC_VER
/**
 * Templated [accelerated] line search descent implementation.
 *
 * @tparam Ff_t `func_functor<In_t, Out_t, Out_d1_t, Out_d2_t>` impl or similar
 *     type with type members defined with `scalar_type`, `gradient_type`,
 *     `hessian_type` type members, ex. by using `PDMATH_USING_FUNCTOR_TYPES`
 * @tparam Ds_t `direction_search<V_t>` impl or similar type implementing
 *     `V_t operator()(const V_t&)` with a `gradient_type` type member, ex. by
 *     as defined by using `PDMATH_USING_CONTAINER_TYPES`
 * @tparam Ss_t `step_search<V_t>` impl or similar type implementing
 *     `element_type operator()(const V_t& x_p, const V_t& dir)`,
 *     `element_type last_step() const`, where `element_type` is
 *     `V_t::value_type`, with `using gradient_type = V_t` type member
 * @tparam Dd_t `direction_policy<V_t>` impl or similar type implementing
 *     `bool operator()(const V_t&)` with a `gradient_type` type memer
 * @tparam Tt_t `std::function`-like taking and returning `V_t` or `const V_t&`
 *
 * @param func `Ff_t` functor giving the unconstained minimization problem
 * @param dir_search `Ds_t` search direction functor used to compute a search
 *     direction from the current guess each iteration
 * @param eta_search `Ss_t` step search functor, which when given the current
 *     guess and search direction returns the step size to take
 * @param x0 `const typename Ff_t::gradient_type&` initial guess
 * @param max_iter `std::uintmax_t` max iterations allowed
 * @param dir_policy `Dp_t` convergence policy returning `true` if the current
 *     search direction meets some [early] convergence criteria
 * @param tail_transform `Tt_t` callable for per-iteration transform of updated
 *     guess after updating previous guess with scaled search direction, ex. a
 *     proximal operator like the soft-thresholding or projection operators
 * @param nesterov `bool` flag, `true` to use Nesterov's momentum scheme
 */
template <
  typename Ff_t, typename Ds_t, typename Ss_t, typename Dp_t, typename Tt_t>
optimize_result<
  typename Ff_t::scalar_type,
  typename Ff_t::gradient_type,
  typename Ff_t::hessian_type>
line_search(
  Ff_t func,
  Ds_t dir_search,
  Ss_t eta_search,
  const typename Ff_t::gradient_type& x0,
  std::uintmax_t max_iter,
  Dp_t dir_policy,
  Tt_t tail_transform,
  bool nesterov = false)
{
  using T = typename Ff_t::scalar_type;
  using V_t = typename Ff_t::gradient_type;
  using M_t = typename Ff_t::hessian_type;
  // check consistency of vector types
  static_assert(std::is_same_v<V_t, typename Ds_t::gradient_type>);
  static_assert(std::is_same_v<V_t, typename Ss_t::gradient_type>);
  static_assert(std::is_same_v<V_t, typename Dp_t::gradient_type>);
  // check that tail transform has the right invocation signature
  static_assert(
    std::is_invocable_r_v<V_t, Tt_t, const V_t&> ||
    std::is_invocable_r_v<V_t, Tt_t, V_t> ||
    std::is_invocable_r_v<const V_t&, Tt_t, const V_t&> ||
    std::is_invocable_r_v<const V_t&, Tt_t, V_t>
  );
  // true when converged, affected by choice of dir_policy
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
// better to pop warning stack for C4706 asap
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
    eta = eta_search((nesterov) ? z : x_c, dx);
    // update the current guess and perform final transform
    std::transform(
      x_c.cbegin(),
      x_c.cend(),
      dx.cbegin(),
      x_c.begin(),
      [&](T x_c_v, T dx_v) { return x_c_v + eta * dx_v; }
    );
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
// MSVC does not like the implicit conversion of n_iter to T here
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(5219)
#endif  // _MSC_VER
          return x_c_v + (n_iter + 1) / (n_iter + 4) * (x_c_v - x_p_v);
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
        }
      );
      x_p = x_c;
    }
    n_iter++;
  }
  return optimize_result<T, V_t, M_t>(
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

/**
 * Templated [accelerated] line search descent without tail transform.
 *
 * @tparam Ff_t `func_functor<In_t, Out_t, Out_d1_t, Out_d2_t>` impl or similar
 *     type with type members defined with `scalar_type`, `gradient_type`,
 *     `hessian_type` type members, ex. by using `PDMATH_USING_FUNCTOR_TYPES`
 * @tparam Ds_t `direction_search<V_t>` impl or similar type implementing
 *     `V_t operator()(const V_t&)` with a `gradient_type` type member, ex. by
 *     as defined by using `PDMATH_USING_CONTAINER_TYPES`
 * @tparam Ss_t `step_search<V_t>` impl or similar type implementing
 *     `element_type operator()(const V_t& x_p, const V_t& dir)`,
 *     `element_type last_step() const`, where `element_type` is
 *     `V_t::value_type`, with `using gradient_type = V_t` type member
 * @tparam Dd_t `direction_policy<V_t>` impl or similar type implementing
 *     `bool operator()(const V_t&)` with a `gradient_type` type memer
 *
 * @param func `Ff_t` functor giving the unconstained minimization problem
 * @param dir_search `Ds_t` search direction functor used to compute a search
 *     direction from the current guess each iteration
 * @param eta_search `Ss_t` step search functor, which when given the current
 *     guess and search direction returns the step size to take
 * @param x0 `const typename Ff_t::gradient_type&` initial guess
 * @param max_iter `std::uintmax_t` max iterations allowed
 * @param dir_policy `Dp_t` convergence policy returning `true` if the current
 *     search direction meets some [early] convergence criteria
 * @param nesterov `bool` flag, `true` to use Nesterov's momentum scheme
 */
template <typename Ff_t, typename Ds_t, typename Ss_t, typename Dp_t>
optimize_result<
  typename Ff_t::scalar_type,
  typename Ff_t::gradient_type,
  typename Ff_t::hessian_type>
line_search(
  Ff_t func,
  Ds_t dir_search,
  Ss_t eta_search,
  const typename Ff_t::gradient_type& x0,
  std::uintmax_t max_iter,
  Dp_t dir_policy,
  bool nesterov = false)
{
  return line_search(
    func,
    dir_search,
    eta_search,
    x0,
    max_iter,
    dir_policy,
    identity_functor<typename Ff_t::gradient_type>(),
    nesterov
  );
}

}  // namespace pdmath

#endif  // PDMATH_LINE_SEARCH_H_
