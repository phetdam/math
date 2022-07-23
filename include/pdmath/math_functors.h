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

namespace pdmath {

/**
 * Templated multivariate quadratic function with defined gradient and Hessian.
 *
 * Letting `H`, `a`, `b`, be the Hessian, affine terms, and scalar translation.
 * Calling `operator()` or `f(x)` evaluates `0.5 * x'Hx + a'x + b`.
 *
 * @tparam T scalar return type
 * @tparam V_t vector input type/gradient return type. Must implement `size`
 *     method, which returns the size, and `begin` and `end` methods that
 *     return the beginning and ending iterators.
 * @tparam M_t Hessian return type. Must implement `size1`, `size2`, number of
 *     rows and columns respectively, `begin1`, `begin2`, `end1`, `end2`, row
 *     and column iterators, respectively, like `boost::matrix`. I.e. the
 *     1 and 2 iterators must also implement `begin`, `end`.
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
    const T& shf = T(0.))
    : shf_(shf)
  {
    assert((aff) ? aff->size() : true);
    assert(hess->size1() && hess->size2() && hess->size1() == hess->size2());
    assert(hess->size2() == aff->size());
    hess_ = hess;
    aff_ = aff;
    n_dims_ = hess->size2();
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
    T res(T(0.5) * std::inner_product(x.begin(), x.end(), h_x.begin(), shf_));
    // if no affine terms, we can just return
    if (!aff_) {
      return res;
    }
    // else we need to add a'x to result
    return res + std::inner_product(x.begin(), x.end(), aff_->begin(), T(0.));
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
      grad.begin(), grad.end(), aff_->begin(), grad.begin(), std::plus<T>()
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
    auto row_itr = hess_->begin1();
    auto res_itr = res.begin();
    while (row_itr != hess_->end1()) {
        *res_itr = std::inner_product(
          row_itr.begin(), row_itr.end(), x.begin(), T(0.)
        );
        row_itr++;
        res_itr++;
    }
    return res;
  }

  std::shared_ptr<V_t> aff_;
  std::shared_ptr<M_t> hess_;
  T shf_;
  std::size_t n_dims_;
};

/**
 * Templated Himmelblau's function.
 */
// template <typename T, typename V_t, typename M_t>
// class himmelblau_functor : public func_functor<V_t, T, V_t, M_t> {
// public:

// };

}  // namespace pdmath

#endif  // PDMATH_MATH_FUNCTORS_H_
