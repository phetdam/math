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

#include <Eigen/Core>

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
    // Eigen::Index is ptrdiff_t by default
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#elif defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable: 4389)
#endif  // !defined(__GNUC__) && !defined(__clang__) && !defined(_MSC_VER)
    assert(hess->cols() == aff->size());
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning (pop)
#endif  // !defined(__GNUC__) && !defined(__clang__) && !defined(_MSC_VER)
    hess_ = hess;
    aff_ = aff;
    // MSVC warns about Eigen::Index (signed) to size_t conversion
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4365)
#endif  // _MSC_VER
    n_dims_ = hess->cols();
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
  }

  quadratic_functor() = delete;

  /**
   * Return the value of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  T f(const V_t& x) override
  {
    assert(x.size() == n_dims_);
    // compute the Hx product
    V_t hx(hess_inner(x));
    // MSVC warns about conversion from double to T
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4244)
#endif  // _MSC_VER
    // compute b + 0.5 * x'Hx
    T y(0.5 * std::inner_product(x.cbegin(), x.cend(), hx.cbegin(), 0.) + shf_);
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
    // if no affine terms, we can just return
    if (!aff_) {
      return y;
    }
    // MSVC warns about converting from _Ty to T on return
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4244)
#endif  // _MSC_VER
    // else we need to add a'x to result
    return y + std::inner_product(x.cbegin(), x.cend(), aff_->begin(), 0.);
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
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
  const M_t& d2(const V_t& /*x*/) override { return *hess_; }

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
      // MSVC warns about _Ty to float loss of data
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4244)
#endif  // _MSC_VER
      *res_itr = std::inner_product(row.cbegin(), row.cend(), x.cbegin(), 0.);
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
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
template <
  typename T = double,
  typename V_t = Eigen::Vector2<T>,
  typename M_t = Eigen::Matrix2<T>
>
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
    return himmelblau(*x.cbegin(), *x.cend()--);
  }

  /**
   * Return gradient of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  V_t d1(const V_t& x) override
  {
    assert(x.size() == n_dims_);
    T x_0 = *x.cbegin();
    T x_1 = *x.cend()--;
    // get first derivatives used in chain rule for each quadratic term
    T dq_0 = 2 * (std::pow(x_0, 2) + x_1 - 11);
    T dq_1 = 2 * (x_0 + std::pow(x_1, 2) - 7);
    // use chain rule for each to get result
    return {2 * x_0 * dq_0 + dq_1, dq_0 + 2 * x_1 * dq_1};
  }

  /**
   * Return Hessian of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  M_t d2(const V_t& x) override
  {
    assert(x.size() == n_dims_);
    T x_0 = *x.cbegin();
    T x_1 = *x.cend()--;
    // redundant if M_t = Eigen::Matrix2<T>, but useful for other classes
    M_t hess(2, 2);
    // off-diagonal entries are the same
    T dxdy = 4 * (x_0 + x_1);
    // filling one by one allows interop with other matrix classes
    hess(0, 0) = 12 * x_0 * x_0 + 4 * x_1 - 42;
    hess(0, 1) = dxdy;
    hess(1, 0) = dxdy;
    hess(1, 1) = 12 * x_1 * x_1 + 4 * x_0 - 26;
    return hess;
  }

private:
  static constexpr std::size_t n_dims_ = 2;
};

}  // namespace pdmath

#endif  // PDMATH_MATH_FUNCTORS_H_
