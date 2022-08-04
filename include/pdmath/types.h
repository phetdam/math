/**
 * @file types.h
 * @author Derek Huang
 * @brief C++ header for some user-defined types/type aliases
 * @copyright MIT License
 */

#ifndef PDMATH_TYPES_H_
#define PDMATH_TYPES_H_

#include <array>
#include <utility>
#include <vector>

#include <Eigen/Core>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

namespace pdmath {

using double_vector = std::vector<double>;

template <typename T>
using boost_vector = boost::numeric::ublas::vector<T>;
template <typename T>
using boost_matrix = boost::numeric::ublas::matrix<T>;

template <typename T>
using array_pair = std::array<T, 2>;

template <typename T>
using eigen_vector = Eigen::Matrix<T, Eigen::Dynamic, 1>;
template <typename T>
using eigen_matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

/**
 * Return `true` if `boost_vector<T>` instances have equal element values.
 */
template <typename T>
inline bool operator==(const boost_vector<T>& a, const boost_vector<T>& b)
{
  if (a.size() != b.size()) {
    return false;
  }
  auto a_itr = a.begin();
  auto b_itr = b.begin();
  while (a_itr != a.end() && b_itr != b.end()) {
    if (*a_itr != *b_itr) {
      return false;
    }
    a_itr++;
    b_itr++;
  }
  return true;
}

/**
 * Return `false` if `boost_vector<T>` instances have equal element values.
 */
template <typename T>
inline bool operator!=(const boost_vector<T>& a, const boost_vector<T>& b)
{
  return !(a == b);
}

/**
 * Return `true` if `boost_matrix<T>` instances have equal element values.
 */
template <typename T>
inline bool operator==(const boost_matrix<T>& a, const boost_matrix<T>& b)
{
  if (a.size1() != b.size1() || a.size2() != b.size2()) {
    return false;
  }
  for (typename boost_matrix<T>::size_type i = 0; i < a.size1(); i++) {
    for (typename boost_matrix<T>::size_type j = 0; j < a.size2(); j++) {
      if (a(i, j) != b(i, j)) {
        return false;
      }
    }
  }
  return true;
}

/**
 * Return `false` if `boost_matrix<T>` instances have equal element values.
 */
template <typename T>
inline bool operator!=(const boost_matrix<T>& a, const boost_matrix<T>& b)
{
  return !(a == b);
}

}  // namespace pdmath

#endif  // PDMATH_TYPES_H_
