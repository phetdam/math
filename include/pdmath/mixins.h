/**
 * @file mixins.h
 * @author Derek Huang <djh458@stern.nyu.edu>
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
 * Used as a mixin in `optimize_result` and its subclasses.
 *
 * @note Non-template base classes cannot have separate implementations if
 *     they are being inherited by templated classes.
 */
class fev_mixin {
public:
  /**
   * `fev_mixin` construtor.
   *
   * @param n_fev `std::uintmax_t` number of function evaluations
   */
  fev_mixin(std::uintmax_t n_fev) : n_fev_(n_fev) {}

  /**
   * Return the function evaluation count.
   */
  std::uintmax_t n_fev() const { return n_fev_; }

private:
  std::uintmax_t n_fev_;
};

/**
 * Class holding gradient and Hessian evaluation counts.
 *
 * Used as a mixin in `scalar_optimize_result` and `vector_optimize_result`.
 *
 * @note Non-template base classes cannot have separate implementations if
 *     they are being inherited by templated classes.
 */
class gev_hev_mixin {
public:
  /**
   * Constructor for `gev_hev_mixin`.
   *
   * @param n_gev `std::uintmax_t` number of gradient evaluations
   * @param n_hev `std::uintmax_t` number of Hessian evaluations
   */
  gev_hev_mixin(std::uintmax_t n_gev, std::uintmax_t n_hev)
    : n_gev_(n_gev), n_hev_(n_hev)
  {}

  /**
   * Getter for `gev_hev_mixin` gradient evaluation count.
   */
  std::uintmax_t n_gev() const { return n_gev_; }

  /**
   * Getter for `gev_hev_mixin` Hessian evaluation count.
   */
  std::uintmax_t n_hev() const { return n_hev_; }

private:
  std::uintmax_t n_gev_;
  std::uintmax_t n_hev_;
};

}  // namespace pdmath

#endif  // PDMATH_MIXINS_H_
