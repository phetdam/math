/**
 * @file math_functors.h
 * @author Derek Huang
 * @brief C++ header for `func_functor` subclasses for mathematical functions
 * @copyright MIT License
 */

#ifndef PDMATH_MATH_FUNCTORS_H_
#define PDMATH_MATH_FUNCTORS_H_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>

#include "pdmath/bases.h"
#include "pdmath/math_functions.h"

namespace pdmath {

/**
 * Templated multivariate quadratic function with defined gradient and Hessian.
 *
 * Letting `H`, `a`, `b`, be the Hessian, affine terms, and scalar translation.
 * Calling `operator()` or `f(x)` evaluates `0.5 * x'Hx + a'x + b`.
 *
 * @tparam T scalar return type
 * @tparam V_t vector input type/gradient return type, a *Container*
 * @tparam M_t Hessian return type, ex. an `Eigen::Matrix` specialization
 */
template <typename T, typename V_t, typename M_t>
class quadratic_functor : public func_functor<V_t, T, V_t, const M_t&> {
public:
  /**
   * `quadratic_functor` full constructor.
   *
   * @param hess `const std::shared_ptr<M_t>&` pointing to `M_t` Hessian
   * @param aff `const std::shared_ptr<V_t>&` pointing to `V_t` affine terms
   * @param shf `const T&` translation term
   */
  quadratic_functor(
    const std::shared_ptr<M_t>& hess,
    const std::shared_ptr<V_t>& aff = nullptr,
    const T& shf = 0.)
    : shf_(shf)
  {
    assert(hess && ((aff) ? aff->size() : true));
    assert(hess->rows() && hess->cols() && hess->rows() == hess->cols());
    assert(hess->cols() == aff->size());
    hess_ = hess;
    aff_ = aff;
    n_dims_ = hess->cols();
  }

  /**
   * Return the value of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  T f(const V_t& x) override
  {
    assert(x.size() == n_dims_);
    // compute the Hx product
    V_t h_x(hess_inner(x));
    // compute b + 0.5 * x'Hx
    T res(0.5 * std::inner_product(x.cbegin(), x.cend(), h_x.cbegin(), shf_));
    // if no affine terms, we can just return
    if (!aff_) {
      return res;
    }
    // else we need to add a'x to result
    return res + std::inner_product(x.begin(), x.end(), aff_->begin(), 0.);
  }

  /**
   * Return gradient of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  V_t d1(const V_t& x) override
  {
    V_t grad(hess_inner(x));
    // if no affine terms, we can just return
    if (!aff_) {
      return grad;
    }
    // else we need to add the affine terms to grad
    std::transform(
      grad.cbegin(), grad.cend(), aff_->cbegin(), grad.begin(), std::plus<T>()
    );
    return grad;
  }

  /**
   * Return Hessian of the function at `x`.
   *
   * No computation is actually done here, we return `const M_t&` to `hess_`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  const M_t& d2(const V_t& x) override { return *hess_; }

  /**
   * Return dimension of the input.
   */
  const std::size_t& n_dims() const { return n_dims_; }

private:
  /**
   * Return inner product of Hessian with some vector `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  V_t hess_inner(const V_t& x)
  {
    assert(x.size() == n_dims_);
    V_t res(n_dims_);
    // compute the Hx product
    auto res_itr = res.begin();
    for (const auto& row : hess_->rowwise()) {
      *res_itr = std::inner_product(row.cbegin(), row.cend(), x.cbegin(), 0.);
      res_itr++;
    }
    // auto row_itr = hess_->begin1();
    // while (row_itr != hess_->end1()) {
    //   *res_itr = std::inner_product(
    //     row_itr.begin(), row_itr.end(), x.cbegin(), 0.
    //   );
    //   row_itr++;
    //   res_itr++;
    // }
    return res;
  }

  std::shared_ptr<V_t> aff_;
  std::shared_ptr<M_t> hess_;
  T shf_;
  std::size_t n_dims_;
};

/**
 * Templated Himmelblau's function with gradient and Hessian.
 */
template <typename T, typename V_t, typename M_t>
class himmelblau_functor : public func_functor<V_t, T, V_t, M_t> {
public:
  /**
   * Return the value of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  T f(const V_t& x) override
  {
    assert(x.size() == n_dims_);
    return himmelblau(*x.begin(), *x.end());
  }

  /**
   * Return gradient of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  V_t d1(const V_t& x) override
  {
    assert(x.size() == n_dims_);
    T x_0 = *x.begin();
    T x_1 = *x.end();
    // get first derivatives used in chain rule for each quadratic term
    T dq_0 = 2 * (std::pow(x_0, 2) + x_1 - 11);
    T dq_1 = 2 * (x_0 + std::pow(x_1, 2) - 7);
    // use chain rule for each to get result
    return {2 * x_0 * dq_0 + dq_1, dq_0 + 2 * x_1 * dq_1};
  }

  // d2 implementation todo

private:
  static constexpr std::size_t n_dims_ = 2;
};

}  // namespace pdmath

#endif  // PDMATH_MATH_FUNCTORS_H_
