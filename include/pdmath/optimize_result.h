/**
 * @file optimize_result.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Template class for a stripped version of the scipy OptimizeResult
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
 * Templated ABC holding optimization results for scalar-valued objectives.
 */
template <class T = double>
class optimize_result : public fev_mixin {
public:
  optimize_result(bool, const std::string&, std::uintmax_t, T, std::uintmax_t);
  bool converged() const;
  const std::string& message() const;
  std::uintmax_t n_iter() const;
  const T& obj() const;
private:
  bool converged_;
  std::string message_;
  std::uintmax_t n_iter_;
  T obj_;
};

/**
 * Constructor for the `optimize_result` base class.
 *
 * @param converged `bool`, `true` if converged, `false` otherwise
 * @param message `const std::string&` message on optimization status
 * @param n_iter `std::uintmax_t` number of iterations taken
 * @param obj `T` value of the objective
 * @param n_fev `std::uintmax_t` number of function evaluations
 */
template <class T>
optimize_result<T>::optimize_result(
  bool converged,
  const std::string& message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev)
  : fev_mixin(n_fev),
    converged_(converged),
    message_(message),
    n_iter_(n_iter),
    obj_(obj)
{}

/**
 * Getter for the `optimize_result` convergence flag.
 */
template <class T>
bool optimize_result<T>::converged() const { return converged_; }

/**
 * Getter for the `optimize_result` convergence message.
 */
template <class T>
const std::string& optimize_result<T>::message() const { return message_; }

/**
 * Getter for the `optimize_result` iteration count.
 */
template <class T>
std::uintmax_t optimize_result<T>::n_iter() const { return n_iter_; }

/**
 * Getter for the `optimize_result` objective value.
 */
template <class T>
const T& optimize_result<T>::obj() const { return obj_; }

/**
 * Templated class holding results of scalar optimization.
 */
template <class T = double>
class scalar_optimize_result
  : public optimize_result<T>, public gev_mixin, public hev_mixin {
public:
  scalar_optimize_result(
    T,
    bool,
    const std::string&,
    std::uintmax_t,
    T,
    std::uintmax_t
  );
  scalar_optimize_result(
    T,
    bool,
    const std::string&,
    std::uintmax_t,
    T,
    std::uintmax_t,
    T,
    std::uintmax_t
  );
  scalar_optimize_result(
    T,
    bool,
    const std::string&,
    std::uintmax_t,
    T,
    std::uintmax_t,
    T,
    std::uintmax_t,
    T,
    std::uintmax_t
  );
  const T& res() const;
  const T& grad() const;
  const T& hess() const;
private:
  T res_;
  T grad_;
  T hess_;
};

/**
 * `scalar_optimize_result` full ctor for univariate optimization results.
 *
 * @param res `T` optimization result
 * @param converged `bool`, `true` if converged, `false` otherwise
 * @param message `const std::string&` message on optimization status
 * @param n_iter `std::uintmax_t` number of iterations taken
 * @param obj `T` value of the objective
 * @param n_fev `std::uintmax_t` number of function evaluations
 * @param grad `T` value of the objective's first derivative (if any)
 * @param n_gev `std::uintmax_t` number of first derivative evaluations
 * @param hess `T` value of the objective's second derivative (if any)
 * @param n_hev `std::uintmax_t` number of second derivative evaluations
 */
template <class T>
scalar_optimize_result<T>::scalar_optimize_result(
  T res,
  bool converged,
  const std::string& message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev,
  T grad,
  std::uintmax_t n_gev,
  T hess,
  std::uintmax_t n_hev)
  : optimize_result<T>(converged, message, n_iter, obj, n_fev),
    gev_mixin(n_gev),
    hev_mixin(n_hev),
    res_(res),
    grad_(grad),
    hess_(hess)
{}

/**
 * `scalar_optimize_result` ctor for 2nd-derivative free methods.
 *
 * See the full constructor for parameter documentation.
 */
template <class T>
scalar_optimize_result<T>::scalar_optimize_result(
  T res,
  bool converged,
  const std::string& message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev,
  T grad,
  std::uintmax_t n_gev)
  : scalar_optimize_result<T>(
      res,
      converged,
      message,
      n_iter,
      obj,
      n_fev,
      grad,
      n_gev,
      std::numeric_limits<T>::quiet_NaN(),
      0)
{}

/**
 * `scalar_optimize_result` ctor for 1st- and 2nd-derivative free methods.
 *
 * See the full constructor for parameter documentation.
 */
template <class T>
scalar_optimize_result<T>::scalar_optimize_result(
  T res,
  bool converged,
  const std::string& message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev)
  : scalar_optimize_result<T>(
      res,
      converged,
      message,
      n_iter,
      obj,
      n_fev,
      std::numeric_limits<T>::quiet_NaN(),
      0,
      std::numeric_limits<T>::quiet_NaN(),
      0)
{}

/**
 * Getter for the `scalar_optimize_result` result.
 */
template <class T>
const T& scalar_optimize_result<T>::res() const { return res_; }

/**
 * Getter for the `scalar_optimize_result` first derivative value.
 */
template <class T>
const T& scalar_optimize_result<T>::grad() const { return grad_; }

/**
 * Getter for the `scalar_optimize_result` second derivative value.
 */
template <class T>
const T& scalar_optimize_result<T>::hess() const { return hess_; }

/**
 * Templated class holding results of multivariate (vector) optimization.
 */
template<
  class T = double, class V_t = boost_vector<T>, class M_t = boost_matrix<T>
>
class vector_optimize_result
  : public optimize_result<T>, public gev_mixin, public hev_mixin {
public:
  vector_optimize_result(
    V_t,
    bool,
    const std::string&,
    std::uintmax_t,
    T,
    std::uintmax_t,
    V_t,
    std::uintmax_t,
    M_t,
    std::uintmax_t
  );
  vector_optimize_result(
    V_t,
    bool,
    const std::string&,
    std::uintmax_t,
    T,
    std::uintmax_t,
    V_t,
    std::uintmax_t
  );
  vector_optimize_result(V_t,
    bool,
    const std::string&,
    std::uintmax_t,
    T,
    std::uintmax_t
  );
  const V_t& res() const;
  const V_t& grad() const;
  const M_t& hess() const;
private:
  V_t res_;
  V_t grad_;
  M_t hess_;
};

/**
 * `vector_optimize_result` constructor for multivariate optimization methods.
 *
 * @param res `V_t` optimization result
 * @param converged `bool`, `true` if converged, `false` otherwise
 * @param message `const std::string&` message on optimization status
 * @param n_iter `std::uintmax_t` number of iterations taken
 * @param obj `T` value of the objective
 * @param n_fev number of function evaluations
 * @param grad `V_t` value of the objective's gradient (if any)
 * @param n_gev number of gradient evaluations
 * @param hess `M_t` value of the objective's Hessian (if any)
 * @param n_hev number of Hessian evaluations
 */
template<class T, class V_t, class M_t>
vector_optimize_result<T, V_t, M_t>::vector_optimize_result(
  V_t res,
  bool converged,
  const std::string& message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev,
  V_t grad,
  std::uintmax_t n_gev,
  M_t hess,
  std::uintmax_t n_hev)
  : optimize_result<T>(converged, message, n_iter, obj, n_fev),
    gev_mixin(n_gev),
    hev_mixin(n_hev),
    res_(res),
    grad_(grad),
    hess_(hess)
{}

/**
 * `vector_optimize_result` ctor for Hessian-free methods.
 *
 * See the full constructor for parameter documentation.
 */
template <class T, class V_t, class M_t>
vector_optimize_result<T, V_t, M_t>::vector_optimize_result(
  V_t res,
  bool converged,
  const std::string& message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev,
  V_t grad,
  std::uintmax_t n_gev)
  : vector_optimize_result<T>(
      res,
      converged,
      message,
      n_iter,
      obj,
      n_fev,
      grad,
      n_gev,
      nullptr,
      0)
{}

/**
 * `vector_optimize_result` ctor for gradient- and Hessian-free methods.
 *
 * See the full constructor for parameter documentation.
 */
template <class T, class V_t, class M_t>
vector_optimize_result<T, V_t, M_t>::vector_optimize_result(
  V_t res,
  bool converged,
  const std::string& message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev)
  : vector_optimize_result<T>(
      res,
      converged,
      message,
      n_iter,
      obj,
      n_fev,
      nullptr,
      0,
      nullptr,
      0)
{}

/**
 * Getter for the `vector_optimize_result` result.
 */
template<class T, class V_t, class M_t>
const V_t& vector_optimize_result<T, V_t, M_t>::res() const { return res_; }

/**
 * Getter for the `vector_optimize_result` gradient value.
 */
template<class T, class V_t, class M_t>
const V_t& vector_optimize_result<T, V_t, M_t>::grad() const { return grad_; }

/**
 * Getter for the `vector_optimize_result` Hessian value.
 */
template<class T, class V_t, class M_t>
const M_t& vector_optimize_result<T, V_t, M_t>::hess() const { return hess_; }

}  // namespace pdmath

#endif  // PDMATH_OPTIMIZE_RESULT_H_
