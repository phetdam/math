/**
 * @file types.h
 * @author Derek Huang
 * @brief C++ header for some user-defined types/type aliases
 * @copyright MIT License
 */

#ifndef PDMATH_TYPES_H_
#define PDMATH_TYPES_H_

#include <utility>
#include <vector>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

namespace pdmath {

using double_vector = std::vector<double>;

template <typename T>
using boost_vector = boost::numeric::ublas::vector<T>;
template <typename T>
using boost_matrix = boost::numeric::ublas::matrix<T>;

/**
 * Return `true` if `boost_vector<T>` instances have equal element values.
 */
template <typename T>
inline bool operator==(const boost_vector<T>& a, const boost_vector<T>& b)
{
  if (a.size() != b.size()) {
    return false;
  }
  for (typename boost_vector<T>::size_type i = 0; i < a.size(); i++) {
    if (a[i] != b[i]) {
      return false;
    }
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
    for (typename boost_matrix<T>:: size_type j = 0; j < a.size2(); j++) {
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
