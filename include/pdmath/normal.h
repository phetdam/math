/**
 * @file normal.h
 * @author Derek Huang
 * @brief C++ header for normal distribution functions
 * @copyright MIT License
 */

#ifndef PDMATH_NORMAL_H_
#define PDMATH_NORMAL_H_

#include <cmath>
#include <type_traits>

#include "pdmath/pi.h"
#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * Functor to represent the univariate standard normal distribution.
 *
 * There is no need to support scaled versions of the normal distribution since
 * given a \f$Z \sim \mathcal{N}(0, 1)\f$, one can always form another normal
 * \f$X \sim \mathcal{N}(\mu, \sigma^2)\f$ with the affine transformation
 * \f$X = \mu + \sigma Z\f$. By also omitting the need to store any parameters,
 * we can define the PDF/CDF of `normal` as customization point objects and
 * therefore delay argument-dependent lookup when appropriate.
 */
class normal {
public:
  /**
   * CPO to represent the normal PDF of the standard normal distribution.
   */
  class pdf_cpo {
  public:
    /**
     * Return the standard normal PDF.
     *
     * @tparam T Floating-point type
     *
     * @param x Evaluation point
     */
    template <typename T, constraint_t<std::is_floating_point_v<T>> = 0>
    constexpr auto operator()(T x) const noexcept
    {
      // TODO: precompute 1 / sqrt(2 * pi_v<T>)
      return T{1} / std::sqrt(2 * pi_v<T>) * std::exp(-x * x / T{2});
    }
  };

  /**
   * CPO to represent the normal CDF of the standard normal distribution.
   */
  class cdf_cpo {
  public:
    /**
     * Return the standard normal CDF.
     *
     * @tparam T Floating-point type
     *
     * @param x Evaluation point
     */
    template <typename T, constraint_t<std::is_floating_point_v<T>> = 0>
    constexpr auto operator()(T x) const noexcept
    {
      // TODO: precompute 1 / sqrt(2)
      return (1 + std::erf(x / std::sqrt(T{2})));
    }
  };

  // members
  static constexpr pdf_cpo pdf{};
  static constexpr cdf_cpo cdf{};
};

}  // namespace pdmath

#endif  // PDMATH_NORMAL_H_
