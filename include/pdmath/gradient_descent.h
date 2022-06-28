/**
 * @file gradient_descent.h
 * @author Derek Huang
 * @brief C++ header for [accelerated] gradient descent implementation
 * @copyright MIT License
 */

#ifndef PDMATH_GRADIENT_DESCENT_H_
#define PDMATH_GRADIENT_DESCENT_H_

#include <cmath>
#include <cstdint>
#include <utility>

#include "pdmath/functor_base.h"
#include "pdmath/optimize_result.h"

namespace pdmath {

/**
 * Base template class for gradient convergence policy functors.
 *
 * When called on the gradient, returns `true` to signal convergence.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 */
template <class T = double, class V_t = bvector_t<T>>
class gradient_policy {
public:
  virtual ~gradient_policy() = default;
  virtual bool operator()(const V_t&) = 0;
};

/**
 * Base template class for vector/matrix norms.
 *
 * @tparam T scalar type
 * @tparam C_t compound type with `T` elements, ex. vector or matrix type
 */
template <class T = double, class C_t = bvector_t<T>>
class norm {
public:
  virtual ~norm() = default;
  virtual T operator()(const C_t&) = 0;
};

/**
 * Template class for the Euclidean norm, i.e. vector 2-norm.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 */
template <class T = double, class V_t = bvector_t<T>>
class euclidean_norm : public norm<T, V_t> {
public:
  /**
   * Return the 2-norm of the vector.
   *
   * @param vector `const V_t&` vector to compute norm for
   */
  T operator()(const V_t& vector)
  {
    T sq_norm();
    for (const auto& value : vector) {
      sq_norm += std::pow(value, 2);
    }
    return std::sqrt(sq_norm);
  }
};

/**
 * Gradient policy where gradient below a minimum implies convergence.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 */
template <class T = double, class V_t = bvector_t<T>>
class min_gradient_norm_policy : public gradient_policy<T, V_t> {
public:
  /**
   * Constructor.
   *
   * @param min_norm `T` minimum gradient norm that must be exceeded during
   *     the routine's execution to prevent early convergence.
   * @param norm `const norm<T, V_t>&` norm functor to compute the norm
   */
  min_gradient_norm_policy(T min_norm, const norm<T, V_t>& norm)
    : min_norm_(min_norm), norm_(norm)
  {}

  /**
   * Return `true` if the norm of `grad` is <= minimium norm.
   *
   * @param grad `const V_t&` gradient vector
   */
  bool operator()(const V_t& grad)
  {
    return norm_(grad) <= min_norm_ ? true : false;
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
 * Templated [accelerated] gradient descent implementation for `std` types.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 * @tparam F_grad gradient callable, `operator()` must take a `V_t`
 * @tparam F_line line search callable, `operator()` must take a
 *     `std::pair<V_t, V_t>` of the current guess and the search direction
 */
template <
  class T = double,
  class V_t = bvector_t<T>,
  class F_grad = functor_base<V_t, T>,
  class F_line = functor_base<std::pair<V_t, V_t>, T>
>
vector_optimize_result<T> gradient_descent(
  F_grad grad,
  F_line line_search,
  const std::uintmax_t& max_iter,
  const gradient_policy<T, V_t>& grad_pol,
  bool nesterov = false)
{
  return vector_optimize_result<T, V_t>(
    V_t(),
    true,
    "converged",
    0,
    T(0),
    0
  );
}

}  // namespace pdmath

#endif  // PDMATH_GRADIENT_DESCENT_H_
