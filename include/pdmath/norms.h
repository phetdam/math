/**
 * @file norms.h
 * @author Derek Huang
 * @brief C++ header for norm functors
 * @copyright MIT License
 */

#ifndef PDMATH_NORMS_H_
#define PDMATH_NORMS_H_

#include <boost/math/tools/norms.hpp>

namespace pdmath {

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
 * Template class for the vector p-norm.
 *
 * When default constructed, defaults to the 2-norm, or Euclidean norm.
 *
 * @tparam T scalar type
 * @tparam V_t vector type, with `T` elements
 */
template <class T = double, class V_t = bvector_t<T>>
class p_norm : public norm<T, V_t> {
public:
  /**
   * Constructor.
   *
   * @param p `unsigned int` value of the `p` used for the p-norm.
   */
  p_norm(unsigned int p = 2) : p_(p) {}

  /**
   * Return the p-norm of the vector.
   *
   * @param vector `const V_t&` vector to compute norm for. Must implement the
   *     `begin` and `end` method to get iterators.
   */
  T operator()(const V_t& vector)
  {
    // T sq_norm();
    // for (const auto& value : vector) {
    //   sq_norm += std::pow(value, 2);
    // }
    // return std::sqrt(sq_norm);
    return boost::math::tools::lp_norm(vector.begin(), vector.end(), p_);
  }

private:
  unsigned int p_;
};

}  // namespace pdmath

#endif  // PDMATH_NORMS_H_
