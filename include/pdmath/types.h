/**
 * @file types.h
 * @author Derek Huang
 * @brief C++ header for some user-defined types/type aliases
 * @copyright MIT License
 */

#ifndef PDMATH_TYPES_H_
#define PDMATH_TYPES_H_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

template <class T>
using bvector_t = boost::numeric::ublas::vector<T>;
template <class T>
using bmatrix_t = boost::numeric::ublas::matrix<T>;

#endif  // PDMATH_TYPES_H_
