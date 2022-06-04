/**
 * @file functor_base.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Header file for a Boost-like functor ABC.
 * @copyright MIT License
 */

#ifndef PDMATH_OPTIMIZE_FUNCTOR_BASE_H_
#define PDMATH_OPTIMIZE_FUNCTOR_BASE_H_

namespace pdmath {
namespace optimize {

template <class T = double>
class functor_base {
public:
  virtual T operator()(const T&) = 0;
};

}  // namespace pdmath::optimize
}  // namespace pdmath

#endif  // PDMATH_OPTIMIZE_FUNCTOR_BASE_H_
