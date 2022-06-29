/**
 * @file bases.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief C++ header for [templated] base classes
 * @copyright MIT License
 */

#ifndef PDMATH_BASES_H_
#define PDMATH_BASES_H_

namespace pdmath {

template <class In_t, class Out_t = In_t>
class functor_base {
public:
  virtual ~functor_base() = default;
  virtual Out_t operator()(const In_t&) = 0;
};

}  // namespace pdmath

#endif  // PDMATH_BASES_H_
