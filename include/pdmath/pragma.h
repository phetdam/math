/**
 * @file pragma.h
 * @author Derek Huang
 * @brief C/C++ header providing abstractions for pragma directives
 * @copyright MIT License
 */

#ifndef PDMATH_PRAGMA_H_
#define PDMATH_PRAGMA_H_

/**
 * Issue a `#pragma` directive without expanding macros.
 *
 * @param x `#pragma` parameters
 */
#define PDMATH_PRAGMA_NOEXPAND(x) _Pragma(#x)

/**
 * Issue a `#pragma` directive with macro expansion.
 *
 * Preferred over `PDMATH_PRAGMA_NOEXPAND` since it can expand passed macros.
 *
 * @param x `#pragma` parameters
 */
#define PDMATH_PRAGMA(x) PDMATH_PRAGMA_NOEXPAND(x)

#endif  // PDMATH_PRAGMA_H_
