/**
 * @file ex4.7.cc
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Main program for exercise 4.7
 * @copyright MIT License
 */

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

#include "pdmath/optimize/functor_base.h"
#include "pdmath/optimize/golden_search.h"
#include "pdmath/optimize/optimize_result.h"

namespace {
/**
 * Functor class for using root-finding to get the group lasso update norm.
 */
template <class T = double>
class group_norm_root_functor : public pdmath::optimize::functor_base<T> {
public:
  /**
   * Constructor for the functor.
   * 
   * Pre-computes the constant term used when calling the functor instance.
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
    : singular_values_(singular_values),
      lambda_(lambda)
  {
    assert(lambda_ > 0);
    assert(singular_values_.size() == proj_residuals.size());
    n_predictors_ = singular_values_.size();
    resid_const_ = T(0);
    for (typename std::vector<T>::size_type i = 0; i < n_predictors_; i++) {
      resid_const_ += pow(singular_values_[i] * proj_residuals[i], 2);
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
      res += std::pow(std::pow(singular_values_[i], 2) * nu + lambda_, 2);
    }
    return res - resid_const_;
  }

private:
  std::vector<T> singular_values_;
  T lambda_;
  T resid_const_;
  typename std::vector<T>::size_type n_predictors_;
};

/**
 * Functor class for using minimization to get the group lasso update norm.
 * 
 * @note Should be made separate from `group_norm_root_functor` later.
 */
template <class T = double>
class group_norm_minimize_functor : public pdmath::optimize::functor_base<T> {
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
  std::vector<double> singular_values({0.4, 0.3, 0.1});
  std::vector<double> proj_residuals({-0.2, 0.35, -0.01});
  // default template type omitted
  group_norm_minimize_functor objective(singular_values, proj_residuals, 0.1);
  pdmath::optimize::scalar_optimize_result res(
    std::move(
      pdmath::optimize::golden_search(
        [](double x) { return std::pow(x, 2); }, -1., 1.
      )
    )
  );
  std::cout << "minimum of x^2: " << res.res() << std::endl;
  std::cout << "converged? " << (res.converged() ? "yes" : "no") << std::endl;
  std::cout << "n_iter: " << res.n_iter() << std::endl;
  std::cout << "n_fev: " << res.n_fev() << std::endl;
  return EXIT_SUCCESS;
}
