/**
 * @file bases.h
 * @author Derek Huang <djh458@stern.nyu.edu>
 * @brief C++ header for [templated] base classes
 * @copyright MIT License
 */

#ifndef PDMATH_BASES_H_
#define PDMATH_BASES_H_

namespace pdmath {

/**
 * Template base class for a general single-argument functor.
 *
 * @tparam In_t input type
 * @tparam Out_t output type, defaults to `In_t`
 */
template <class In_t, class Out_t = In_t>
class functor_base {
public:
  virtual ~functor_base() = default;
  virtual Out_t operator()(const In_t&) = 0;
};

/**
 * Template functor for identity function.
 */
template <class T>
class identity_functor : public functor_base<T, T&> {
  T& operator()(const T& o) { return o; }
};

}  // namespace pdmath

#endif  // PDMATH_BASES_H_
