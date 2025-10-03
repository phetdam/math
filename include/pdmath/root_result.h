/**
 * @file root_result.h
 * @author Derek Huang
 * @brief Template class implementing the SciPy RootResults
 * @copyright MIT License
 */

#ifndef PDMATH_ROOT_RESULT_H_
#define PDMATH_ROOT_RESULT_H_

#include <cstdint>
#include <string>

#include "pdmath/mixins.h"

namespace pdmath {

/**
 * Templated class holding results of univariate root finding.
 *
 * @tparam T Result type
 */
template <typename T>
class root_result : public fev_mixin {
public:
  /**
   * `root_result` constructor.
   *
   * @param root `const T&` root estimate
   * @param converged `bool` `true` if converged, `false` otherwise
   * @param message `const std::string&` convergence message
   * @param n_iter `std::uintmax_t` number of iterations
   * @param n_fev `std::uintmax_t` number of function evaluations
   */
  root_result(
    const T& root,
    bool converged,
    const std::string& message,
    std::uintmax_t n_iter,
    std::uintmax_t n_fev)
    : fev_mixin(n_fev),
      root_(root),
      converged_(converged),
      message_(message),
      n_iter_(n_iter)
  {}

  root_result() = delete;

  const T& root() const { return root_; }
  bool converged() const { return converged_; }
  const std::string& message() const { return message_; }
  std::uintmax_t n_iter() const { return n_iter_; }

private:
  T root_;
  bool converged_;
  std::string message_;
  std::uintmax_t n_iter_;
};

}  // namespace pdmath

#endif  // PDMATH_ROOT_RESULT_H_
