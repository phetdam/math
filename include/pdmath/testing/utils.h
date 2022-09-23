/**
 * @file utils.h
 * @author Derek Huang
 * @brief C++ header for some testing utils
 * @copyright MIT License
 */

#ifndef PDMATH_TESTING_UTILS_H_
#define PDMATH_TESTING_UTILS_H_

namespace pdmath {
namespace testing {

/**
 * Templated wrapper for scalar, gradient, and Hessian types.
 *
 * Specializations can be used with the Google Test `::testing::Types` template
 * for templated tests with `TYPED_TEST` in Google Test.
 *
 * @tparam T scalar return type
 * @tparam V_t vector input type/gradient return type, a `*Container*
 * @tparam M_t Hessian return type, ex. an `Eigen::Matrix` specialization
 */
template <typename T, typename V_t, typename M_t>
class func_type_triple {
public:
  using scalar_t = T;
  using vector_t = V_t;
  using matrix_t = M_t;
  func_type_triple() = delete;
};

}  // namespace pdmath::testing
}  // namespace pdmath

#endif  // PDMATH_TESTING_UTILS_H_
