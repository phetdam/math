/**
 * @file macros.h
 * @author Derek Huang
 * @brief C++ header for some macros shared by pdmath unit tests
 * @copyright MIT License
 */

#ifndef PDMATH_TESTING_MACROS_H_
#define PDMATH_TESTING_MACROS_H_

// [approximate] minimizers of Himmelblau's function that make the function
// zero. these can be used in initializer lists, as function arguments, etc.
#define HIMMELBLAU_ZERO_1 3, 2
#define HIMMELBLAU_ZERO_2 -2.805118, 3.131312
#define HIMMELBLAU_ZERO_3 -3.779310, -3.283186
#define HIMMELBLAU_ZERO_4 3.584428, -1.848126

#endif  // PDMATH_TESTING_MACROS_H_
