/**
 * @file bases.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief C++ header for [templated] base classes
 * @copyright MIT License
 */

#ifndef PDMATH_BASES_H_
#define PDMATH_BASES_H_

namespace pdmath {

/**
 * Template base class for a general single-argument functor.
 *
 * @tparam In_t input type
 * @tparam Out_t output type, defaults to `In_t`
 */
template <typename In_t, typename Out_t = In_t>
class functor_base {
public:
  virtual ~functor_base() = default;
  virtual Out_t operator()(const In_t&) = 0;
};

/**
 * Template functor for identity function.
 */
template <typename T>
class identity_functor : public functor_base<T, T&> {
  T& operator()(const T& o) { return o; }
};

/**
 * Template functor to represent a function, up to twice differentiable.
 *
 * @tparam In_t input type
 * @tparam Out_t output type, defaults to `In_t`
 * @tparam Out_d1_t first deriv output type, defaults to `Out_t`
 * @tparam Out_d2_t second deriv output type, defaults to `Out_t`
 * @tparam F_f function, takes `const In_t&`, return `Out_t`
 * @tparam F_d1 first deriv, takes `const In_t&`, return `Out_d1_t`
 * @tparam F_d2 second deriv, takes `const In_t&`, return `Out_d2_t`
 */
template <
  typename In_t,
  typename Out_t = In_t,
  typename Out_d1_t = Out_t,
  typename Out_d2_t = Out_t,
  typename F_f = functor_base<In_t, Out_t>,
  typename F_d1 = functor_base<In_t, Out_d1_t>,
  typename F_d2 = functor_base<In_t, Out_d2_t>
>
class function_functor : public functor_base<In_t, Out_t> {
public:
  /**
   * `function_functor` constructor.
   *
   * @param f `F_f` function
   * @param d1 `F_d1` first derivative, ex. gradient
   * @param d2 `F_d2` second derivative, ex. Hessian
   */
  function_functor(
    F_f f,
    F_d1 d1 = [](const In_t& x) { return Out_d1_t(); },
    F_d2 d2 = [](const In_t& x) { return Out_d2_t(); })
    : f_(f), d1_(d1), d2_(d2)
  {}

  Out_t operator()(const In_t& x) { return f(); }

  /**
   * Return result of evaluating the function.
   */
  Out_t f(const In_t& x) const { return f_(x); }

  /**
   * Return result of evaluating the derivative.
   */
  Out_d1_t d1(const In_t& x) const { return d1_(x); }

  /**
   * Return result of evaluating the Hessian.
   */
  Out_d2_t d2(const In_t& x) const { return d2_(x); }

private:
  F_f f_;
  F_d1 d1_;
  F_d2 d2_;
};

}  // namespace pdmath

#endif  // PDMATH_BASES_H_
