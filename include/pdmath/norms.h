/**
 * @file norms.h
 * @author Derek Huang
 * @brief C++ header for norm functors
 * @copyright MIT License
 */

#ifndef PDMATH_NORMS_H_
#define PDMATH_NORMS_H_

#include <boost/math/tools/norms.hpp>

#include "pdmath/types.h"

namespace pdmath {

/**
 * Base template class for vector/matrix norms.
 *
 * @tparam T scalar type
 * @tparam C_t compound type with `T` elements, ex. vector or matrix type
 */
template <class T = double, class C_t = boost_vector<T>>
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
template <class T = double, class V_t = boost_vector<T>>
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
    // need to handle p_ == 0 case separately, as lp_norm gives infinity. also
    // use separate cases for other Boost norms, since they are faster.
    if (!p_) {
      return boost::math::tools::l0_pseudo_norm(vector);
    }
    else if (p_ == 1) {
      return boost::math::tools::l1_norm(vector);
    }
    else if (p_ == 2) {
      return boost::math::tools::l2_norm(vector);
    }
    return boost::math::tools::lp_norm(vector, p_);
  }

private:
  unsigned int p_;
};

}  // namespace pdmath

#endif  // PDMATH_NORMS_H_
