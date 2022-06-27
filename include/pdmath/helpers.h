/**
 * @file helpers.h
 * @author Derek Huang
 * @brief C++ header for project-specific helper macros
 * @copyright MIT License
 */

#ifndef PDMATH_HELPERS_H_
#define PDMATH_HELPERS_H_

#include <iostream>
#include <string>

namespace pdmath {

/**
 * Print a short banner header with the file name of the example program run.
 *
 * Useful for identifying the program in `examples` being run by the user.
 *
 * @param filename `const char *` name of the program with `main` in `examples`
 */
inline void print_example_header(const char *filename)
{
  std::string filename_(filename);
  std::string split_str("math/examples");
  auto split_idx = filename_.find(split_str);
  // the +1 at the end is to not include the directory slash
  std::cout << "[" << filename_.substr(split_idx + split_str.size() + 1) << "]";
  std::cout << std::endl;
}

}  // namespace pdmath

#endif  // PDMATH_HELPERS_H_