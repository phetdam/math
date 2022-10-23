/**
 * @file warnings.h
 * @author Derek Huang
 * @brief C/C++ header providing abstractions for handling compiler warnings
 * @copyright MIT License
 */

#ifndef PDMATH_WARNINGS_H_
#define PDMATH_WARNINGS_H_

#include "pdmath/pragma.h"

#if defined(_MSC_VER)

/**
 * Save current warning state, restorable by using `PDMATH_WARNINGS_POP`.
 */
#define PDMATH_WARNINGS_PUSH() PDMATH_PRAGMA(warning (push))

/**
 * Retrieve previously saved warning state and overwrite current warning state.
 */
#define PDMATH_WARNINGS_POP() PDMATH_PRAGMA(warning (pop))

/**
 * Disable one or several MSVC warnings.
 *
 * @param wnos MSVC warning number(s), ex. 4305
 */
#define PDMATH_WARNINGS_DISABLE(wnos) PDMATH_PRAGMA(warning (disable: wnos))

/**
 * Treat one or several MSVC warnings as error(s).
 *
 * @param wnos MSVC warning number(s), ex. 4305
 */
#define PDMATH_WARNINGS_ERROR(wnos) PDMATH_PRAGMA(warning (error: wnos))

/**
 * Show the warning only once for one or several MSVC warnings.
 *
 * @param wnos MSVC warning number(s), ex. 4305
 */
#define PDMATH_WARNINGS_ONCE(wnos) PDMATH_PRAGMA(warning (once: wnos))

// uses __GNUC__ instead of __GNUG__ to allow use with C code
#elif defined(__GNUC__) || defined(__clang__)

/**
 * Save current warning state, restorable by using `PDMATH_WARNINGS_POP`.
 */
#define PDMATH_WARNINGS_PUSH() PDMATH_PRAGMA(GCC diagnostic push)

/**
 * Retrieve previously saved warning state and overwrite current warning state.
 */
#define PDMATH_WARNINGS_POP() PDMATH_PRAGMA(GCC diagnostic pop)

/**
 * Disable a GCC/Clang warning.
 *
 * @param ws Warning string literal, ex. "-Wuninitialized"
 */
#define PDMATH_WARNINGS_DISABLE(ws) PDMATH_PRAGMA(GCC diagnostic ignored ws)

/**
 * Treat a GCC/Clang warning as an error.
 *
 * @param ws Warning string literal, ex. "-Wuninitialized"
 */
#define PDMATH_WARNINGS_ERROR(ws) PDMATH_PRAGMA(GCC diagnostic error ws)

#else

#define PDMATH_WARNINGS_PUSH()
#define PDMATH_WARNINGS_POP()
#define PDMATH_WARNINGS_DISABLE(ws)
#define PDMATH_WARNINGS_ERROR(ws)
#warning "unknown compiler: PDMATH_WARNINGS_* macros defined as empty"

#endif  // !defined(_MSC_VER) && !defined(__GNUC__) && !defined(__clang__)

#endif  // PDMATH_WARNINGS_H_
