/**
 * @file common.h
 * @author Derek Huang
 * @brief C/C++ common macros
 * @copyright MIT License
 */

#ifndef PDMATH_COMMON_H_
#define PDMATH_COMMON_H_

// __cplusplus macro that works even with MSVC
#if defined(_MSVC_LANG)
#define PDMATH_CPLUSPLUS _MSVC_LANG
#elif defined(__cplusplus)
#define PDMATH_CPLUSPLUS __cplusplus
#endif  // !defined(_MSVC_LANG) && !defined(__cplusplus)

/**
 * Stringify argument without macro expansion.
 *
 * @param x Identifier to stringify
 */
#define PDMATH_STRINGIFY_I(x) #x

/**
 * Stringify argument with macro expansion.
 *
 * @param x Identifier to stringify
 */
#define PDMATH_STRINGIFY(x) PDMATH_STRINGIFY_I(x)

/**
 * Concatenate arguments without macro expansion.
 *
 * @param a First argument
 * @param b Second argument
 */
#define PDMATH_CONCAT_I(a, b) a ## b

/**
 * Concatenate arguments with macro expansion.
 *
 * @param a First argument
 * @param b Second argument
 */
#define PDMATH_CONCAT(a, b) PDMATH_CONCAT_I(a, b)

#endif  // PDMATH_COMMON_H_
