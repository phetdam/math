/**
 * @file optimize_result.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Template class for a stripped version of the scipy OptimizeResult
 * @copyright MIT License
 */

#ifndef PDMATH_OPTIMIZE_OPTIMIZE_RESULT_H_
#define PDMATH_OPTIMIZE_OPTIMIZE_RESULT_H_

#include <cstdint>
#include <limits>
#include <string>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

namespace pdmath {
namespace optimize {

template <class T>
using bvector_t = boost::numeric::ublas::vector<T>;
template <class T>
using bmatrix_t = boost::numeric::ublas::matrix<T>;

/**
 * Class holding function evaluation counts.
 * 
 * Used as a mixin in `optimize_result` and its subclasses.
 * 
 * @note Non-template base classes cannot have separate implementations if
 *     they are being inherited by templated classes.
 */
class fev_mixin {
public:
  fev_mixin(std::uintmax_t);
  std::uintmax_t n_fev() const;
private:
  std::uintmax_t n_fev_;
};

/**
 * Constructor for `fev_mixin`.
 * 
 * @param n_fev `std::uintmax_t` number of function evaluations
 */
fev_mixin::fev_mixin(std::uintmax_t n_fev) : n_fev_(n_fev) {}

/**
 * Getter for `fev_mixin` function evaluation count.
 */
std::uintmax_t fev_mixin::n_fev() const { return n_fev_; }

/**
 * Class holding gradient and Hessian evaluation counts.
 * 
 * Used as a mixin in `scalar_optimize_result` and `vector_optimize_result`.
 */
class gev_hev_mixin {
public:
  gev_hev_mixin(std::uintmax_t, std::uintmax_t);
  std::uintmax_t n_gev() const;
  std::uintmax_t n_hev() const;
private:
  std::uintmax_t n_gev_;
  std::uintmax_t n_hev_;
};

/**
 * Constructor for `gev_hev_mixin`.
 * 
 * @param n_gev `std::uintmax_t` number of gradient evaluations
 * @param n_hev `std::uintmax_t` number of Hessian evaluations
 */
gev_hev_mixin::gev_hev_mixin(std::uintmax_t n_gev, std::uintmax_t n_hev)
  : n_gev_(n_gev), n_hev_(n_hev)
{}

/**
 * Getter for `gev_hev_mixin` gradient evaluation count.
 */
std::uintmax_t gev_hev_mixin::n_gev() const { return n_gev_; }

/**
 * Getter for `gev_hev_mixin` Hessian evaluation count.
 */
std::uintmax_t gev_hev_mixin::n_hev() const { return n_hev_; }

/**
 * Templated ABC holding optimization results for scalar-valued objectives.
 */
template <class T = double>
class optimize_result : public fev_mixin {
public:
  optimize_result(bool, std::string, std::uintmax_t, T, std::uintmax_t);
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
 * @param message `std::string` message on optimization status
 * @param n_iter `std::uintmax_t` number of iterations taken
 * @param obj `T` value of the objective
 * @param n_fev `std::uintmax_t` number of function evaluations
 */
template <class T>
optimize_result<T>::optimize_result(
  bool converged,
  std::string message,
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
class scalar_optimize_result : public optimize_result<T>, public gev_hev_mixin {
public:
  scalar_optimize_result(
    T,
    bool,
    std::string,
    std::uintmax_t,
    T,
    std::uintmax_t
  );
  scalar_optimize_result(
    T,
    bool,
    std::string,
    std::uintmax_t,
    T,
    std::uintmax_t,
    T,
    std::uintmax_t
  );
  scalar_optimize_result(
    T,
    bool,
    std::string,
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
 * @param message `std::string` message on optimization status
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
  std::string message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev,
  T grad,
  std::uintmax_t n_gev,
  T hess,
  std::uintmax_t n_hev)
  : optimize_result<T>(converged, message, n_iter, obj, n_fev),
    gev_hev_mixin(n_gev, n_hev),
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
  std::string message,
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
  std::string message,
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
  class T = double,
  class vector_t = bvector_t<T>,
  class matrix_t = bmatrix_t<T>
>
class vector_optimize_result : public optimize_result<T>, public gev_hev_mixin {
public:
  vector_optimize_result(
    vector_t,
    bool,
    std::string,
    std::uintmax_t,
    T,
    std::uintmax_t,
    vector_t,
    std::uintmax_t,
    matrix_t,
    std::uintmax_t
  );
  vector_optimize_result(
    vector_t,
    bool,
    std::string,
    std::uintmax_t,
    T,
    std::uintmax_t,
    vector_t,
    std::uintmax_t
  );
  vector_optimize_result(
    vector_t,
    bool,
    std::string,
    std::uintmax_t,
    T,
    std::uintmax_t
  );
  const vector_t& res() const;
  const vector_t& grad() const;
  const matrix_t& hess() const;
private:
  vector_t res_;
  vector_t grad_;
  matrix_t hess_;
};

/**
 * `vector_optimize_result` constructor for multivariate optimization methods.
 * 
 * @param res `vector_t` optimization result
 * @param converged `bool`, `true` if converged, `false` otherwise
 * @param message `std::string` message on optimization status
 * @param n_iter `std::uintmax_t` number of iterations taken
 * @param obj `T` value of the objective
 * @param n_fev number of function evaluations
 * @param grad `vector_t` value of the objective's gradient (if any)
 * @param n_gev number of gradient evaluations
 * @param hess `matrix_t` value of the objective's Hessian (if any)
 * @param n_hev number of Hessian evaluations
 */
template<class T, class vector_t, class matrix_t>
vector_optimize_result<T, vector_t, matrix_t>::vector_optimize_result(
  vector_t res,
  bool converged,
  std::string message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev,
  vector_t grad,
  std::uintmax_t n_gev,
  matrix_t hess,
  std::uintmax_t n_hev)
  : optimize_result<T>(converged, message, n_iter, obj, n_fev),
    gev_hev_mixin(n_gev, n_hev),
    res_(res),
    grad_(grad),
    hess_(hess)
{}

/**
 * `vector_optimize_result` ctor for Hessian-free methods.
 * 
 * See the full constructor for parameter documentation.
 */
template <class T, class vector_t, class matrix_t>
vector_optimize_result<T, vector_t, matrix_t>::vector_optimize_result(
  vector_t res,
  bool converged,
  std::string message,
  std::uintmax_t n_iter,
  T obj,
  std::uintmax_t n_fev,
  vector_t grad,
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
template <class T, class vector_t, class matrix_t>
vector_optimize_result<T, vector_t, matrix_t>::vector_optimize_result(
  vector_t res,
  bool converged,
  std::string message,
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
template<class T, class vector_t, class matrix_t>
const vector_t& vector_optimize_result<T, vector_t, matrix_t>::res() const
{
  return res_;
}

/**
 * Getter for the `vector_optimize_result` gradient value.
 */
template<class T, class vector_t, class matrix_t>
const vector_t& vector_optimize_result<T, vector_t, matrix_t>::grad() const
{
  return grad_;
}

/**
 * Getter for the `vector_optimize_result` Hessian value.
 */
template<class T, class vector_t, class matrix_t>
const matrix_t& vector_optimize_result<T, vector_t, matrix_t>::hess() const
{
  return hess_;
}

} // namespace pdmath::optimize
} // namespace pdmath

#endif // PDMATH_OPTIMIZE_OPTIMIZE_RESULT_H_
