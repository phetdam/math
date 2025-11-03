/**
 * @file impl_policy.h
 * @author Derek Huang
 * @brief C++ header for implementation policy objects
 * @copyright MIT License
 */

#ifndef PDMATH_IMPL_POLICY_H_
#define PDMATH_IMPL_POLICY_H_

namespace pdmath {

/**
 * Serial or standard implementation policy type.
 *
 * This represents a typical serial implementation.
 */
struct serial_implementation {};

/**
 * Global object to differentiate serial implementations.
 */
inline constexpr serial_implementation serial;

/**
 * SIMD implementation policy type.
 *
 * This represents a special SIMD implementation.
 */
struct simd_implementation {};

/**
 * Global object to differentiate SIMD implementations.
 */
inline constexpr simd_implementation simd;

}  // namespace pdmath

#endif  // PDMATH_IMPL_POLICY_H_
