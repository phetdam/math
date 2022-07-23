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
 * Overloads `operator()` so the object is callable.
 *
 * @tparam In_t input type
 * @tparam Out_t output type, defaults to `In_t`
 * @tparam Out_d1_t first deriv output type, defaults to `Out_t`
 * @tparam Out_d2_t second deriv output type, defaults to `Out_t`
 */
template <
  typename In_t,
  typename Out_t = In_t,
  typename Out_d1_t = Out_t,
  typename Out_d2_t = Out_t
>
class func_functor : public functor_base<In_t, Out_t> {
public:
  virtual ~func_functor() = default;

  Out_t operator()(const In_t& x) { return f(); }

  /**
   * Return result of evaluating the function.
   */
  virtual Out_t f(const In_t& x) = 0;

  /**
   * Return result of evaluating the derivative.
   */
  virtual Out_d1_t d1(const In_t& x) { return Out_d1_t(); }

  /**
   * Return result of evaluating the Hessian.
   */
  virtual Out_d2_t d2(const In_t& x) { return Out_d2_t(); }
};

}  // namespace pdmath

#endif  // PDMATH_BASES_H_
