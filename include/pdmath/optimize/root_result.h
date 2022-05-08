/**
 * @file root_result.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Template class implementing the SciPy RootResults
 * @copyright MIT License
 */

#ifndef PDMATH_OPTIMIZE_ROOT_RESULT_H_
#define PDMATH_OPTIMIZE_ROOT_RESULT_H_

#include <cstdint>
#include <string>

#include "pdmath/optimize/fev_mixin.h"

namespace pdmath {
namespace optimize {

/**
 * Templated class holding results of univariate root finding.
 */
template<class T = double>
class root_result : public fev_mixin {
public:
  root_result(T, bool, std::string, std::uintmax_t, std::uintmax_t);
  const T& root() const;
  bool converged() const;
  const std::string& message() const;
  std::uintmax_t n_iter() const;
private:
  T root_;
  bool converged_;
  std::string message_;
  std::uintmax_t n_iter_;
  std::uintmax_t n_fev_;
};

/**
 * Constructor for the `root_result`.
 */
template<class T>
root_result<T>::root_result(
  T root,
  bool converged,
  std::string message,
  std::uintmax_t n_iter,
  std::uintmax_t n_fev)
  : fev_mixin(n_fev),
    root_(root),
    converged_(converged),
    message_(message),
    n_iter_(n_iter)
{}

/**
 * Getter for `root_result` root.
 */
template<class T>
const T& root_result<T>::root() const { return root_; }

/**
 * Getter for `root_result` convergence flag.
 */
template<class T>
bool root_result<T>::converged() const { return converged_; }

/**
 * Getter for `root_result` convergence message.
 */
template<class T>
const std::string& root_result<T>::message() const { return message_; }

/**
 * Getter for `root_result` number of root-finder iterations.
 */
template<class T>
std::uintmax_t root_result<T>::n_iter() const { return n_iter_; }

}  // namespace pdmath::optimize
}  // namespace pdmath

#endif  // PDMATH_OPTIMIZE_ROOT_RESULT_H_
