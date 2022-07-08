/**
 * @file optimize_result.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief C++ header implementing a stripped SciPy OptimizeResult template
 * @copyright MIT License
 */

#ifndef PDMATH_OPTIMIZE_RESULT_H_
#define PDMATH_OPTIMIZE_RESULT_H_

#include <cstdint>
#include <limits>
#include <string>

#include "pdmath/mixins.h"
#include "pdmath/types.h"

namespace pdmath {

/**
 * Templated class holding optimization results for scalar-valued objectives.
 *
 * @tparam T objective function return type
 * @tparam X_t optimization variable type
 * @tparam G_t optimization gradient/first derivative type
 * @tparam H_t optimization Hessian/second derivative type
 */
template <class T, class X_t = T, class G_t = X_t, class H_t = X_t>
class optimize_result : public fev_mixin, public gev_mixin, public hev_mixin {
public:
  /**
   * `optimize_result` full constructor.
   *
   * @param res `X_t` optimization result
   * @param converged `bool`, `true` if converged, `false` otherwise
   * @param message `const std::string&` message on optimization status
   * @param n_iter `std::uintmax_t` number of iterations taken
   * @param obj `T` value of the objective
   * @param n_fev number of function evaluations
   * @param grad `G_t` value of the objective's gradient (if any)
   * @param n_gev number of gradient evaluations
   * @param hess `H_t` value of the objective's Hessian (if any)
   * @param n_hev number of Hessian evaluations
   */
  optimize_result(
    const X_t& res,
    bool converged,
    const std::string& message,
    std::uintmax_t n_iter,
    T obj,
    std::uintmax_t n_fev,
    const G_t& grad,
    std::uintmax_t n_gev,
    const H_t& hess,
    std::uintmax_t n_hev)
    : fev_mixin(n_fev),
      gev_mixin(n_gev),
      hev_mixin(n_hev),
      res_(res),
      converged_(converged),
      message_(message),
      n_iter_(n_iter),
      obj_(obj),
      grad_(grad),
      hess_(hess)
  {}

  /**
   * `optimize_result` constructor for gradient-only methods.
   *
   * See `optimize_result` full constructor for parameter documentation.
   */
  optimize_result(
    const X_t& res,
    bool converged,
    const std::string& message,
    std::uintmax_t n_iter,
    T obj,
    std::uintmax_t n_fev,
    const G_t& grad,
    std::uintmax_t n_gev)
    : optimize_result(
        res, converged, message, n_iter, obj, n_fev, grad, n_gev, H_t(), 0)
  {}

  /**
   * `optimize_result` constructor for derivative-free methods.
   *
   * See `optimize_result` full constructor for parameter documentation.
   */
  optimize_result(
    const X_t& res,
    bool converged,
    const std::string& message,
    std::uintmax_t n_iter,
    T obj,
    std::uintmax_t n_fev)
    : optimize_result(res, converged, message, n_iter, obj, n_fev, G_t(), 0)
  {}

  /**
   * Return  the `optimize_result` convergence flag.
   */
  bool converged() const { return converged_; }

  /**
   * Return the `optimize_result` convergence message.
   */
  const std::string& message() const { return message_; }

  /**
   * Return the `optimize_result` iteration count.
   */
  std::uintmax_t n_iter() const { return n_iter_; }

  /**
   * Return the `optimize_result` objective value.
   */
  const T& obj() const { return obj_; }

  /**
   * Return the `optimize_result` result.
   */
  const X_t& res() const { return res_; }

  /**
   * Return the `optimize_result` gradient/1st derivative value.
   */
  const G_t& grad() const { return grad_; }

  /**
   * Return the `optimize_result` Hessian/2nd derivative value.
   */
  const H_t& hess() const { return hess_; }

private:
  X_t res_;
  bool converged_;
  std::string message_;
  std::uintmax_t n_iter_;
  T obj_;
  G_t grad_;
  H_t hess_;
};

}  // namespace pdmath

#endif  // PDMATH_OPTIMIZE_RESULT_H_
