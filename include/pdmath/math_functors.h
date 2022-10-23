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
#include <type_traits>

#include <Eigen/Core>

#include "pdmath/bases.h"
#include "pdmath/helpers.h"
#include "pdmath/math_functions.h"
#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * Templated multivariate quadratic function with defined gradient and Hessian.
 *
 * Letting `H`, `a`, `b`, be the Hessian, affine terms, and scalar translation.
 * Calling `operator()` or `f(x)` evaluates `0.5 * x'Hx + a'x + b`.
 *
 * @tparam V_t vector input type/gradient return type, a *Container*
 * @tparam M_t Hessian return type, ex. an `Eigen::Matrix` specialization
 */
template <typename V_t, typename M_t>
class quadratic_functor
  : public func_functor<V_t, typename V_t::value_type, V_t, const M_t&> {
public:
  PDMATH_USING_FUNCTOR_TYPES(V_t, M_t);

  /**
   * `quadratic_functor` full constructor.
   *
   * @param hess `const std::shared_ptr<M_t>&` pointing to `M_t` Hessian
   * @param aff `const std::shared_ptr<V_t>&` pointing to `V_t` affine terms
   * @param shf `const scalar_type&` translation term
   */
  quadratic_functor(
    const std::shared_ptr<M_t>& hess,
    const std::shared_ptr<V_t>& aff = nullptr,
    const scalar_type& shf = 0.)
    : shf_(shf)
  {
    assert(hess && ((aff) ? aff->size() : true));
    assert(hess->rows() && hess->cols() && hess->rows() == hess->cols());
// Eigen::Index is ptrdiff_t by default. we can use PDMATH_WARNINGS_(PUSH|POP)
// without #ifdef, as PDMATH_WARNINGS_* macros are empty if compiler unknown.
PDMATH_WARNINGS_PUSH()
#if defined(__GNUG__) || defined(__clang__)
PDMATH_WARNINGS_DISABLE("-Wsign-compare")
#elif defined(_MSC_VER)
PDMATH_WARNINGS_DISABLE(4389)
#endif  // !defined(__GNUG__) && !defined(__clang__) && !defined(_MSC_VER)
    assert(hess->cols() == aff->size());
PDMATH_WARNINGS_POP()
    hess_ = hess;
    aff_ = aff;
// MSVC warns about Eigen::Index (signed) to size_t conversion
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4365)
#endif  // _MSC_VER
    n_dims_ = hess->cols();
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
  }

  quadratic_functor() = delete;

  /**
   * Return the value of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  scalar_type f(const V_t& x) override
  {
// g++/MSVC warns about integer signedness when using Eigen::Matrix
// specializations, as they use a signed integer for the matrix size
PDMATH_WARNINGS_PUSH()
#if defined(__GNUG__)
PDMATH_WARNINGS_DISABLE("-Wsign-compare")
#elif defined(_MSC_VER)
PDMATH_WARNINGS_DISABLE(4389)
#endif  // !defined(__GNUG__) && !defined(_MSC_VER)
    assert(x.size() == n_dims_);
PDMATH_WARNINGS_POP()
    // compute the Hx product
    V_t hx(hess_inner(x));
// MSVC warns about conversion from double to T
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4244)
#endif  // _MSC_VER
    // compute b + 0.5 * x'Hx
    scalar_type y = 0.5 *
      std::inner_product(x.cbegin(), x.cend(), hx.cbegin(), 0.) +
      shf_;
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
    // if no affine terms, we can just return
    if (!aff_)
      return y;
// MSVC warns about converting from _Ty to T on return
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4244)
#endif  // _MSC_VER
    // else we need to add a'x to result
    return y + std::inner_product(x.cbegin(), x.cend(), aff_->begin(), 0.);
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
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
      grad.cbegin(),
      grad.cend(),
      aff_->cbegin(),
      grad.begin(),
      std::plus<scalar_type>()
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
// g++/MSVC warns about integer signedness when using Eigen::Matrix
// specializations, as they use a signed integer for the matrix size
PDMATH_WARNINGS_PUSH()
#if defined(__GNUG__)
PDMATH_WARNINGS_DISABLE("-Wsign-compare")
#elif defined(_MSC_VER)
PDMATH_WARNINGS_DISABLE(4389)
#endif  // !defined(__GNUG__) && !defined(_MSC_VER)
    assert(x.size() == n_dims_);
PDMATH_WARNINGS_POP()
    // supports std::array, which has a fixed size
    V_t res;
    // re-allocate vector types or Container types that have this constructor
    if constexpr (is_vector_size_constructible_v<V_t>) { res = V_t(n_dims_); }
    // compute the Hx product
    auto res_itr = res.begin();
    for (const auto& row : hess_->rowwise()) {
// MSVC warns about _Ty to float loss of data
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4244)
#endif  // _MSC_VER
      *res_itr = std::inner_product(row.cbegin(), row.cend(), x.cbegin(), 0.);
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
      res_itr++;
    }
    return res;
  }

  std::shared_ptr<V_t> aff_;
  std::shared_ptr<M_t> hess_;
  scalar_type shf_;
  std::size_t n_dims_;
};

/**
 * Templated Himmelblau's function with gradient and Hessian.
 *
 * The 4 zeros are (3, 2), approx. (-2.805118, 3.131312), approx.
 * (-3.779310, -3.283186), and approx. (3.584428, -1.848126).
 *
 * @tparam V_t vector input type/gradient return type, a *Container*
 * @tparam M_t Hessian return type, ex. an `Eigen::Matrix` specialization
 */
template <typename V_t = Eigen::Vector2d, typename M_t = Eigen::Matrix2d>
class himmelblau_functor
  : public func_functor<V_t, typename V_t::value_type, V_t, M_t> {
public:
  PDMATH_USING_FUNCTOR_TYPES(V_t, M_t);

  /**
   * Return the value of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  scalar_type f(const V_t& x) override
  {
    assert(x.size() == n_dims_);
    return himmelblau(x[0], x[1]);
  }

  /**
   * Return gradient of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  V_t d1(const V_t& x) override
  {
    assert(x.size() == n_dims_);
    scalar_type x_0 = x[0];
    scalar_type x_1 = x[1];
// MSVC warns about converting from double to T
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4244)
#endif  // _MSC_VER
    // get first derivatives used in chain rule for each quadratic term
    scalar_type dq_0 = 2 * (std::pow(x_0, 2) + x_1 - 11);
    scalar_type dq_1 = 2 * (x_0 + std::pow(x_1, 2) - 7);
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
    // use chain rule for each to get result
    return V_t{{2 * x_0 * dq_0 + dq_1, dq_0 + 2 * x_1 * dq_1}};
  }

  /**
   * Return Hessian of the function at `x`.
   *
   * @param x `const V_t&` point to evaluate at
   */
  M_t d2(const V_t& x) override
  {
    assert(x.size() == n_dims_);
    scalar_type x_0 = x[0];
    scalar_type x_1 = x[1];
    // redundant if M_t = Eigen::Matrix2<scalar_type>, useful for other classes
    M_t hess(2, 2);
    // off-diagonal entries are the same
    scalar_type dxdy = 4 * (x_0 + x_1);
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
