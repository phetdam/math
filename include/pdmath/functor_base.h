/**
 * @file functor_base.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief Header file for a Boost-like functor ABC.
 * @copyright MIT License
 */

#ifndef PDMATH_FUNCTOR_BASE_H_
#define PDMATH_FUNCTOR_BASE_H_

namespace pdmath {

template <class In_t = double, class Out_t = In_t>
class functor_base {
public:
  virtual ~functor_base() = default;
  virtual Out_t operator()(const In_t&) = 0;
};

}  // namespace pdmath

#endif  // PDMATH_FUNCTOR_BASE_H_
