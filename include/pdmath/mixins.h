/**
 * @file mixins.h
 * @author Derek Huang
 * @brief C++ headers for [templated] mixin classes
 * @copyright MIT License
 */

#ifndef PDMATH_MIXINS_H_
#define PDMATH_MIXINS_H_

#include <cstdint>

namespace pdmath {

/**
 * Class holding function evaluation counts.
 *
 * Used as a mixin in `optimize_result` and its subclasses or any class that
 * needs a `n_fev` getter with modifiable `n_fev_` member.
 *
 * @note Non-template base classes cannot have separate implementations if
 *     they are being inherited by templated classes.
 */
class fev_mixin {
public:
  /**
   * `fev_mixin` constructor.
   *
   * @param n_fev `std::uintmax_t` number of function evaluations
   */
  fev_mixin(std::uintmax_t n_fev = 0) : n_fev_(n_fev) {}

  /**
   * Return the function evaluation count.
   */
  std::uintmax_t n_fev() const { return n_fev_; }

protected:
  std::uintmax_t n_fev_;
};

/**
 * Class holding gradient evaluation counts.
 *
 * Used similarly to `fev_mixin`, but for gradient evaluation counts.
 */
class gev_mixin {
public:
  /**
   * `gev_mixin` constructor.
   *
   * @param n_gev `std::uintmax_t` number of gradient evaluations
   */
  gev_mixin(std::uintmax_t n_gev = 0) : n_gev_(n_gev) {}

  /**
   * Return the gradient evaluation count.
   */
  std::uintmax_t n_gev() const { return n_gev_; }

protected:
  std::uintmax_t n_gev_;
};

/**
 * Class holding Hessian evaluation counts.
 *
 * Used similarly to `fev_mixin`, but for Hessian evaluation counts.
 */
class hev_mixin {
public:
  /**
   * `hev_mixin` constructor.
   *
   * @param n_hev `std::uintmax_t` number of Hessian evaluations
   */
  hev_mixin(std::uintmax_t n_hev = 0) : n_hev_(n_hev) {}

  /**
   * Return the Hessian evaluation count.
   */
  std::uintmax_t n_hev() const { return n_hev_; }

protected:
  std::uintmax_t n_hev_;
};

}  // namespace pdmath

#endif  // PDMATH_MIXINS_H_
