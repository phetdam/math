/**
 * @file warnings.h
 * @author Derek Huang
 * @brief C/C++ header providing abstractions for handling compiler warnings
 * @copyright MIT License
 */

#ifndef PDMATH_WARNINGSS_H_
#define PDMATH_WARNINGSS_H_

#include "pdmath/pragma.h"

#if defined(_MSC_VER)

/**
 * Save current warning state, restorable by using `PDMATH_WARNINGS_POP`.
 */
#define PDMATH_WARNINGS_PUSH PDMATH_PRAGMA(warning (push))

/**
 * Retrieve previously saved warning state and overwrite current warning state.
 */
#define PDMATH_WARNINGS_POP PDMATH_PRAGMA(warning (pop))

/**
 * Disable one or several MSVC warnings.
 *
 * @param msvc_warning_numbers MSVC warning number(s), ex. 4305
 */
#define PDMATH_WARNINGS_DISABLE(msvc_warning_numbers) \
  PDMATH_PRAGMA(warning (disable: msvc_warning_numbers))

/**
 * Treat one or several MSVC warnings as error(s).
 *
 * @param msvc_warning_numbers MSVC warning number(s), ex. 4305
 */
#define PDMATH_WARNINGS_ERROR(msvc_warning_numbers) \
  PDMATH_PRAGMA(warning (error: msvc_warning_numbers))

/**
 * Show the warning only once for one or several MSVC warnings.
 *
 * @param msvc_warning_numbers MSVC warning number(s), ex. 4305
 */
#define PDMATH_WARNINGS_ONCE(msvc_warning_numbers) \
  PDMATH_PRAGMA(warning (once: msvc_warning_numbers ))

// uses __GNUC__ instead of __GNUG__ to allow use with C code
#elif defined(__GNUC__) || defined(__clang__)

/**
 * Save current warning state, restorable by using `PDMATH_WARNINGS_POP`.
 */
#define PDMATH_WARNINGS_PUSH PDMATH_PRAGMA(GCC diagnostic push)

/**
 * Retrieve previously saved warning state and overwrite current warning state.
 */
#define PDMATH_WARNINGS_POP PDMATH_PRAGMA(GCC diagnostic pop)

/**
 * Disable a GCC/Clang warning.
 *
 * @param gnu_warning Warning string literal, ex. "-Wuninitialized"
 */
#define PDMATH_WARNINGS_DISABLE(gnu_warning) \
  PDMATH_PRAGMA(GCC diagnostic ignored gnu_warning)

/**
 * Treat a GCC/Clang warning as an error.
 *
 * @param gnu_warning Warning string literal, ex. "-Wuninitialized"
 */
#define PDMATH_WARNINGS_ERROR(gnu_warning) \
  PDMATH_PRAGMA(GCC diagnostic error gnu_warning)

#else

#define PDMATH_WARNINGS_PUSH
#warning "unknown compiler: PDMATH_WARNINGS_PUSH defined as empty"
#define PDMATH_WARNINGS_POP
#warning "unknown compiler: PDMATH_WARNINGS_POP defined as empty"
#define PDMATH_WARNINGS_DISABLE(ignored)
#warning "unknown compiler: PDMATH_WARNINGS_DISABLE defined as empty"
#define PDMATH_WARNINGS_ERROR(ignored)
#warning "unknown compiler: PDMATH_WARNINGS_ERROR defined as empty"

#endif  // !defined(_MSC_VER) && !defined(__GNUC__) && !defined(__clang__)

#endif  // PDMATH_WARNINGSS_H_
