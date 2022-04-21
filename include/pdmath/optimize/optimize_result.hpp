/**
 * @file optimize_result.hpp
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Template class for a stripped version of the scipy OptimizeResult
 * @copyright MIT License
 */

#ifndef PDMATH_OPTIMIZE_OPTIMIZE_RESULT_H_
#define PDMATH_OPTIMIZE_OPTIMIZE_RESULT_H_

#include <cstdint>
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
 * Templated class holding results of multivariate (vector) optimization.
 */
template<
  class T = double,
  class vector_t = bvector_t<T>,
  class matrix_t = bmatrix_t<T>
>
class optimize_result {
public:
  optimize_result(
    vector_t,
    bool,
    std::string,
    std::uintmax_t,
    T,
    vector_t = nullptr,
    matrix_t = nullptr
  );
  const vector_t& res() const;
  bool converged() const;
  const std::string& message() const;
  const std::uintmax_t n_iter() const;
  const T& obj() const;
  const vector_t& grad() const;
  const matrix_t& hess() const;
private:
  vector_t res_;
  bool converged_;
  std::string message_;
  std::uintmax_t n_iter_;
  T obj_;
  vector_t grad_;
  matrix_t hess_;
};

/**
 * `optimize_result` constructor for scalar optimization methods.
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
optimize_result<T, vector_t, matrix_t>::optimize_result(
  vector_t res,
  bool converged,
  std::string message,
  std::uintmax_t n_iter,
  T obj,
  vector_t grad,
  matrix_t hess)
  : res_(res),
    converged_(converged),
    message_(message),
    n_iter_(n_iter),
    obj_(obj),
    grad_(grad),
    hess_(hess)
{}

/**
 * Getter for the `optimize_result` result.
 */
template<class T, class vector_t, class matrix_t>
const vector_t& optimize_result<T, vector_t, matrix_t>::res() const
{
  return res_;
}

/**
 * Getter for the `optimize_result` convergence flag.
 */
template<class T, class vector_t, class matrix_t>
bool optimize_result<T, vector_t, matrix_t>::converged() const
{
  return converged_;
}

/**
 * Getter for the `optimize_result` convergence message.
 */
template<class T, class vector_t, class matrix_t>
const std::string& optimize_result<T, vector_t, matrix_t>::message() const
{
  return message_;
}

/**
 * Getter for the `optimize_result` iteration count.
 */
template<class T, class vector_t, class matrix_t>
const std::uintmax_t optimize_result<T, vector_t, matrix_t>::n_iter() const
{
  return n_iter_;
}

/**
 * Getter for the `optimize_result` objective value.
 */
template<class T, class vector_t, class matrix_t>
const T& optimize_result<T, vector_t, matrix_t>::obj() const { return obj; }

/**
 * Getter for the `optimize_result` gradient value.
 */
template<class T, class vector_t, class matrix_t>
const vector_t& optimize_result<T, vector_t, matrix_t>::grad() const
{
  return grad_;
}

/**
 * Getter for the `optimize_result` Hessian value.
 */
template<class T, class vector_t, class matrix_t>
const matrix_t& optimize_result<T, vector_t, matrix_t>::hess() const
{
  return hess_;
}

} // namespace pdmath::optimize
} // namespace pdmath

#endif // PDMATH_OPTIMIZE_OPTIMIZE_RESULT_H_
