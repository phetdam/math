/**
 * @file helpers.h
 * @author Derek Huang
 * @brief C++ header for project-specific helper macros
 * @copyright MIT License
 */

#ifndef PDMATH_HELPERS_H_
#define PDMATH_HELPERS_H_

#include <cstddef>
#include <iostream>
#include <initializer_list>
#include <string>

#include "pdmath/types.h"

namespace pdmath {

/**
 * Print a short banner header with the file name of the example program run.
 *
 * Useful for identifying the program in `examples` being run by the user.
 *
 * @param file_path `const char *` path of the program with `main` in `examples`
 * @param print `bool` where the header is printed if `true`
 * @returns `std::string` giving the header to print
 */
inline std::string print_example_header(
  const char *file_path, bool print = true)
{
  std::string abs_path(file_path);
  std::string split_str("math/examples");
  // the +1 at the end is to not include the directory slash
  std::string rel_path = abs_path.substr(
    abs_path.find(split_str) + split_str.size() + 1
  );
  std::string frame(rel_path.size(), '-');
  std::string header = "+" + frame + "+\n|" + rel_path + "|\n+" + frame + "+";
  if (print) {
    std::cout << header << std::endl;
  }
  return header;
}

/**
 * Create a new Boost vector from a `std::vector`.
 *
 * @tparam T `boost_vector` element type
 *
 * @param from `const std::vector<T>&` vector whose elements we will copy
 */
template <class T>
inline boost_vector<T> new_boost_vector(const std::vector<T>& from)
{
  boost_vector<T> to(from.size());
  for (std::size_t i = 0; i < from.size(); i++) {
    to[i] = from[i];
  }
  return to;
}

/**
 * Create a new Boost vector from a `std::initializer_list`.
 *
 * Braced list, i.e. `{ ... }` to `from`, will also work.
 *
 * @tparam T `boost_vector` element type
 *
 * @param from `const std::initializer_list<T>&` initialization list
 */
template <class T>
inline boost_vector<T> new_boost_vector(const std::initializer_list<T>& from)
{
  return new_boost_vector(std::vector<T>(from));
}

}  // namespace pdmath

#endif  // PDMATH_HELPERS_H_
