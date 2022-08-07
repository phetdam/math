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
#include <functional>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

#include "pdmath/bases.h"
#include "pdmath/golden_search.h"
#include "pdmath/helpers.h"
#include "pdmath/optimize_result.h"
#include "pdmath/types.h"

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
    numer_coefs_ = std::vector<T>(n_predictors_);
    denom_coefs_ = std::vector<T>(n_predictors_);
    std::transform(
      singular_values.begin(),
      singular_values.end(),
      proj_residuals.begin(),
      numer_coefs_.begin(),
      [](const T& s, const T& pr) { return std::pow(s * pr, 2); }
    );
    std::transform(
      singular_values.begin(),
      singular_values.end(),
      denom_coefs_.begin(),
      [](const T& s) { return std::pow(s, 2); }
    );
  }

  /**
   * Evaluate for a value of the norm.
   *
   * @param nu Norm of the kth class weight vector
   * @returns `T` difference between term with `nu` and constant term
   */
  T operator()(const T& nu)
  {
    return std::transform_reduce(
      numer_coefs_.cbegin(),
      numer_coefs_.cend(),
      denom_coefs_.cbegin(),
      T(-1.),
      // reduce method, just sums up the values
      std::plus<T>(),
      // transform method, computes all the summation terms
      [&](const T& nc, const T& dc)
      {
        return nc / std::pow(dc * nu + lambda_, 2);
      }
    );
  }

private:
  std::vector<T> numer_coefs_;
  std::vector<T> denom_coefs_;
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
  // lambda used in the objective
  double lam = 0.1;
  // singular values and projected residuals + their max and min values
  pdmath::double_vector svs = {0.47, 0.3, 0.1};
  pdmath::double_vector prs = {-0.2, 0.35, -0.01};
  double smin = *std::min_element(svs.begin(), svs.end());
  double smax = *std::max_element(svs.begin(), svs.end());
  double rmax = *std::max_element(prs.begin(), prs.end());
  // objective functor initialized with singular values, proj residuals, lambda
  group_norm_minimize_functor objective(svs, prs, lam);
  // bounds that bracket the solution
  auto bounds = std::make_pair<double, double>(
    0,
    std::sqrt(smax * rmax * svs.size()) /
    std::pow(smin, 2)
  );
  // contains the norm of the group coefficients
  auto res = pdmath::golden_search(objective, bounds.first, bounds.second);
  // print results
  pdmath::print_example_header(__FILE__);
  std::cout << "lambda: " << lam << std::endl;
  std::cout << "singular values: ";
  pdmath::print_vector(svs);
  std::cout << "projected residuals: ";
  pdmath::print_vector(prs);
  std::cout << "bounds: (" << bounds.first << ", " << bounds.second << ")";
  std::cout << std::endl;
  std::cout << "target norm: " << res.res() << std::endl;
  return EXIT_SUCCESS;
}
