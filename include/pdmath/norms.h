/**
 * @file norms.h
 * @author Derek Huang
 * @brief C++ header for norm functors
 * @copyright MIT License
 */

#ifndef PDMATH_NORMS_H_
#define PDMATH_NORMS_H_

// needed for std::isfinite which boost/math/tools/norms.hpp uses unqualified
#include <cmath>

// boost/math/tools/norms.hpp from Boost 1.74.0 seems to have forgotten to
// indicate which isfinite() is being looked up so we introduce to namespace
namespace boost::math::tools {

using std::isfinite;

}  // namespace boost::math::tools

#include <Eigen/Core>
#include <boost/math/tools/norms.hpp>

#include "pdmath/helpers.h"
#include "pdmath/types.h"

namespace pdmath {

/**
 * Base template class for vector/matrix norms.
 *
 * @tparam C_t *Container* type, ex. vector or matrix type
 */
template <typename C_t>
class norm {
public:
  PDMATH_USING_CONTAINER_TYPES(C_t);
  virtual ~norm() = default;
  virtual element_type operator()(const C_t&) = 0;
};

/**
 * Template class for the vector p-norm.
 *
 * When default constructed, defaults to the 2-norm, or Euclidean norm.
 *
 * @tparam V_t *Container* type representing a vector
 */
template <typename V_t = Eigen::VectorXd>
class p_norm : public norm<V_t> {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);

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
  element_type operator()(const V_t& vector) override
  {
    // need to handle p_ == 0 case separately, as lp_norm gives infinity. also
    // use separate cases for other Boost norms, since they are faster.
    if (!p_) {
      // need to convert returned size_t to element_type
      return static_cast<element_type>(boost::math::tools::l0_pseudo_norm(vector));
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

/**
 * Template class implementation for the max (infinity) norm.
 *
 * @tparam V_t *Container* type reprsenting a vector
 */
template <typename V_t = Eigen::VectorXd>
class max_norm : public norm<V_t> {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);

  /**
   * Return max norm of `x`.
   */
  element_type operator()(const V_t& x) override
  {
    return std::abs(*std::max_element(x.cbegin(), x.cend(), max_comp));
  }

private:
  /**
   * Comparison function used by `std::max_element` in `operator()`.
   *
   * @param a `const T&` first value
   * @param b `const T&` second value
   * @returns `true` if `std::abs(a) < std::abs(b)` else `false`
   */
  static bool max_comp(const element_type& a, const element_type& b)
  {
    return std::abs(a) < std::abs(b);
  }
};

}  // namespace pdmath

#endif  // PDMATH_NORMS_H_
