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

template<class T>
using bvector_t = boost::numeric::ublas::vector<T>;
template<class T>
using bmatrix_t = boost::numeric::ublas::matrix<T>;

/**
 * Templated ABC holding optimization results for scalar-valued objectives.
 */
template <class T = double>
class optimize_result {
public:
  optimize_result(bool, std::string, std::uintmax_t, T);
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
 */
template <class T>
optimize_result<T>::optimize_result(
  bool converged,
  std::string message,
  std::uintmax_t n_iter,
  T obj)
  : converged_(converged), message_(message), n_iter_(n_iter), obj_(obj)
{}

/**
 * Getter for the `optimize_result` convergence flag.
 */
template<class T>
bool optimize_result<T>::converged() const { return converged_; }

/**
 * Getter for the `optimize_result` convergence message.
 */
template<class T>
const std::string& optimize_result<T>::message() const { return message_; }

/**
 * Getter for the `optimize_result` iteration count.
 */
template<class T>
std::uintmax_t optimize_result<T>::n_iter() const { return n_iter_; }

/**
 * Getter for the `optimize_result` objective value.
 */
template<class T>
const T& optimize_result<T>::obj() const { return obj_; }

/**
 * Templated class holding results of scalar optimization.
 */
template<class T = double>
class scalar_optimize_result : public optimize_result<T> {
public:
  scalar_optimize_result(
    T,
    bool,
    std::string,
    std::uintmax_t,
    T,
    T = T(std::numeric_limits<double>::quiet_NaN()),
    T = T(std::numeric_limits<double>::quiet_NaN())
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
 * `scalar_optimize_result` constructor for univariate optimization methods.
 * 
 * @param res `T` optimization result
 * @param converged `bool`, `true` if converged, `false` otherwise
 * @param message `std::string` message on optimization status
 * @param n_iter `std::uintmax_t` number of iterations taken
 * @param obj `T` value of the objective
 * @param grad `T` value of the objective's first derivative (if any)
 * @param hess `T` value of the objective's second derivative (if any)
 */
template <class T>
scalar_optimize_result<T>::scalar_optimize_result(
  T res,
  bool converged,
  std::string message,
  std::uintmax_t n_iter,
  T obj,
  T grad,
  T hess)
  : optimize_result<T>(converged, message, n_iter, obj),
    res_(res),
    grad_(grad),
    hess_(hess)
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
class vector_optimize_result : public optimize_result<T> {
public:
  vector_optimize_result(
    vector_t,
    bool,
    std::string,
    std::uintmax_t,
    T,
    vector_t = nullptr,
    matrix_t = nullptr
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
 * @param grad `vector_t` value of the objective's gradient (if any)
 * @param hess `matrix_t` value of the objective's Hessian (if any)
 */
template<class T, class vector_t, class matrix_t>
vector_optimize_result<T, vector_t, matrix_t>::vector_optimize_result(
  vector_t res,
  bool converged,
  std::string message,
  std::uintmax_t n_iter,
  T obj,
  vector_t grad,
  matrix_t hess)
  : optimize_result<T>(converged, message, n_iter, obj),
    res_(res),
    grad_(grad),
    hess_(hess)
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
