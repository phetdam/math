/**
 * @file bases.h
 * @author Derek Huang
 * @brief C++ header for [templated] base classes
 * @copyright MIT License
 */

#ifndef PDMATH_BASES_H_
#define PDMATH_BASES_H_

namespace pdmath {

/**
 * Template base class for a single-argument functor.
 *
 * @tparam In_t input type, exposed as `input_type`
 * @tparam Out_t output type, defaults to `In_t`, exposed as `output_type`
 */
template <typename In_t, typename Out_t = In_t>
class functor_base {
public:
  using input_type = In_t;
  using output_type = Out_t;
  virtual ~functor_base() = default;
  virtual Out_t operator()(const In_t&) = 0;
};

/**
 * Template functor for identity function.
 */
template <typename T>
class identity_functor : public functor_base<T, const T&> {
public:
  const T& operator()(const T& o) override { return o; }
};

/**
 * Template functor to represent a function, up to twice differentiable.
 *
 * @tparam In_t input type, exposed as `input_type`
 * @tparam Out_t output type, exposed as `output_type`
 * @tparam Out_d1_t first deriv output type, exposed as `gradient_type`
 * @tparam Out_d2_t second deriv output type, exposed as `hessian_type`
 */
template <typename In_t, typename Out_t, typename Out_d1_t, typename Out_d2_t>
class func_functor : public functor_base<In_t, Out_t> {
public:
  using input_type = In_t;
  using output_type = Out_t;
  using gradient_type = Out_d1_t;
  using hessian_type = Out_d2_t;

  virtual ~func_functor() = default;

  Out_t operator()(const In_t& x) override { return f(x); }

  /**
   * Return result of evaluating the function.
   */
  virtual Out_t f(const In_t& x) = 0;

  /**
   * Return result of evaluating the derivative.
   */
  virtual Out_d1_t d1(const In_t& x) = 0;

  /**
   * Return result of evaluating the Hessian.
   */
  virtual Out_d2_t d2(const In_t& x) = 0;
};

}  // namespace pdmath

#endif  // PDMATH_BASES_H_
