/**
 * @file ex4.7.cc
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Main program for exercise 4.7
 * @copyright MIT License
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

#include "pdmath/functor_base.h"
#include "pdmath/golden_search.h"
#include "pdmath/optimize_result.h"

namespace {

/**
 * Functor class for using root-finding to get the group lasso update norm.
 */
template <class T = double>
class group_norm_root_functor : public pdmath::functor_base<T> {
public:
  /**
   * Constructor for the functor.
   *
   * Pre-computes the constant coefficients used when calling the functor
   * instance, namely the squared Hadamard product of `singular_values` and
   * `proj_residuals` as well as the squared values of `singular_values`.
   *
   * @param singular_values Vector of positive singular values from the
   *    predictor group matrix. Length is number of group predictors `d_k`,
   *    which assumes that the group predictor matrix is full rank.
   * @param proj_residuals Vector of current prediction residuals for the group
   *    projected onto the columns of the `N` by `d_k` matrix with orthonormal
   *    columns resulting from the [compact] SVD of the predictor group matrix.
   *    Length of `proj_residuals` is therefore also `d_k`.
   * @param lambda The positive group lasso regularization parameter.
   */
  group_norm_root_functor(
    std::vector<T>& singular_values,
    std::vector<T>& proj_residuals,
    T lambda)
    : lambda_(lambda)
  {
    assert(lambda_ > 0);
    assert(singular_values.size() == proj_residuals.size());
    n_predictors_ = singular_values.size();
    numerator_coefs_ = std::vector<T>(n_predictors_);
    denominator_coefs_ = std::vector<T>(n_predictors_);
    for (typename std::vector<T>::size_type i = 0; i < n_predictors_; i++) {
      numerator_coefs_[i] = std::pow(singular_values[i] * proj_residuals[i], 2);
      denominator_coefs_[i] = std::pow(singular_values[i], 2);
    }
  }

  /**
   * Evaluate for a value of the norm.
   *
   * @param nu Norm of the kth class weight vector
   * @returns `T` difference between term with `nu` and constant term
   */
  T operator()(const T& nu)
  {
    T res(0.);
    for (typename std::vector<T>::size_type i = 0; i < n_predictors_; i++) {
      res +=
        numerator_coefs_[i] / std::pow(denominator_coefs_[i] * nu + lambda_, 2);
    }
    return res - T(1.);
  }

private:
  std::vector<T> numerator_coefs_;
  std::vector<T> denominator_coefs_;
  T lambda_;
  typename std::vector<T>::size_type n_predictors_;
};

/**
 * Functor class for using minimization to get the group lasso update norm.
 *
 * @note Should be made separate from `group_norm_root_functor` later.
 */
template <class T = double>
class group_norm_minimize_functor : public pdmath::functor_base<T> {
public:
  /**
   * Constructor for the functor.
   *
   * @param singular_values Vector of positive singular values from the
   *    predictor group matrix. Length is number of group predictors `d_k`,
   *    which assumes that the group predictor matrix is full rank.
   * @param proj_residuals Vector of current prediction residuals for the group
   *    projected onto the columns of the `N` by `d_k` matrix with orthonormal
   *    columns resulting from the [compact] SVD of the predictor group matrix.
   *    Length of `proj_residuals` is therefore also `d_k`.
   * @param lambda The positive group lasso regularization parameter.
   */
  group_norm_minimize_functor(
    std::vector<T>& singular_values,
    std::vector<T>& proj_residuals,
    T lambda)
    : diff_functor_(singular_values, proj_residuals, lambda)
  {}

  /**
   * Evaluate for a value of the norm.
   *
   * @param nu Norm of the kth class weight vector
   * @returns `T` difference between term with `nu` and constant term
   */
  T operator()(const T& nu) { return std::pow(diff_functor_(nu), 2); }

private:
  group_norm_root_functor<T> diff_functor_;
};

}  // namespace

int main()
{
  double lam = 0.1;
  std::vector<double> singular_values({0.47, 0.3, 0.1});
  std::vector<double> proj_residuals({-0.2, 0.35, -0.01});
  double singular_min =
    *std::min_element(singular_values.begin(), singular_values.end());
  double singular_max =
    *std::max_element(singular_values.begin(), singular_values.end());
  double residuals_max =
    *std::max_element(proj_residuals.begin(), proj_residuals.end());
  group_norm_minimize_functor objective(singular_values, proj_residuals, lam);
  auto bounds = std::make_pair<double, double>(
    0,
    std::sqrt(singular_max * residuals_max * singular_values.size()) /
      std::pow(singular_min, 2)
  );
  auto res = pdmath::golden_search(objective, bounds.first, bounds.second);
  std::cout << "lambda: " << lam << std::endl;
  std::cout << "singular values:";
  for (const auto& value : singular_values) {
    std::cout << " " << value;
  }
  std::cout << std::endl;
  std::cout << "projected residuals:";
  for (const auto& value : proj_residuals) {
    std::cout << " " << value;
  }
  std::cout << std::endl;
  std::cout << "bounds: (" << bounds.first << ", " << bounds.second << ")";
  std::cout << std::endl;
  std::cout << "target norm: " << res.res() << std::endl;
  return EXIT_SUCCESS;
}
