/**
 * @file fev_mixin.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Mixin class for class that holds function evaluation counts
 * @copyright MIT License
 */

#ifndef PDMATH_OPTIMIZE_FEV_MIXIN_H_
#define PDMATH_OPTIMIZE_FEV_MIXIN_H_

namespace pdmath {
namespace optimize {

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
  fev_mixin(std::uintmax_t);
  std::uintmax_t n_fev() const;
private:
  std::uintmax_t n_fev_;
};

/**
 * Constructor for `fev_mixin`.
 * 
 * @param n_fev `std::uintmax_t` number of function evaluations
 */
fev_mixin::fev_mixin(std::uintmax_t n_fev) : n_fev_(n_fev) {}

/**
 * Getter for `fev_mixin` function evaluation count.
 */
std::uintmax_t fev_mixin::n_fev() const { return n_fev_; }

} // namespace pdmath::optimize
} // namespace pdmath

#endif // PDMATH_OPTIMIZE_FEV_MIXIN_H_
