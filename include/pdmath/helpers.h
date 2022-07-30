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
#include <sstream>
#include <string>
#include <utility>

#include "pdmath/bases.h"
#include "pdmath/types.h"

namespace pdmath {

/**
 * Print a short banner header with the file name of the example program run.
 *
 * Useful for identifying the program in `examples` being run by the user.
 *
 * @param path `const std::string&` path of program with `main` in `examples`.
 *     This should be an absolute path, i.e. with use of `__FILE__`.
 * @param print `bool` where the header is printed with an extra `\n` if `true`
 * @returns `std::string` giving the header to print
 */
inline std::string print_example_header(
  const std::string& path, bool print = true)
{
  std::string split_str("math/examples");
  // the +1 at the end is to not include the directory slash
  std::string rpath = path.substr(path.find(split_str) + split_str.size() + 1);
  std::string frame(rpath.size(), '-');
  std::string header = "+" + frame + "+\n|" + rpath + "|\n+" + frame + "+";
  if (print) {
    std::cout << header << std::endl;
  }
  return header;
}

/**
 * Print a short banner header with the file name of the example program run.
 *
 * Accepts a C string (null-terminated `char` array) instead of `std::string`.
 *
 * @param path `const char *` path of the program with `main` in `examples`.
 *     This should be an absolute path, i.e. with use of `__FILE__`.
 * @param print `bool` where the header is printed with an extra `\n` if `true`
 * @returns `std::string` giving the header to print
 */
inline std::string print_example_header(const char *path, bool print = true)
{
  return print_example_header(std::string(path), print);
}

/**
 * Controls how `print_vector` prints `std::vector<T>` instances.
 */
class vector_print_policy {
public:
  /**
   * `vector_print_policy` full constructor.
   *
   * @param delim `const std::string&` token used to separate vector values
   * @param padding `unsigned int` number of spaces to print after `delim`
   *     before printing the next value in the vector
   * @param pre_token `const std::string&` token printed before the values
   * @param pre_padding `unsigned int` number of spaces to print after
   *     `pre_token` before printing the first vector value
   * @param post_token `const std::string&` token printed after the values
   * @param post_padding `unsigned int` number of spaces to print before
   *     `post_token` after printing the last vector value
   */
  vector_print_policy(
    const std::string& delim,
    unsigned int padding,
    const std::string& pre_token,
    unsigned int pre_padding,
    const std::string& post_token,
    unsigned int post_padding)
    : delim_(delim),
      padding_(padding),
      pre_token_(pre_token),
      pre_padding_(pre_padding),
      post_token_(post_token),
      post_padding_(post_padding)
  {}

  /**
   * Simplified constructor with no pre/post padding or tokens.
   *
   * See the full constructor for parameter documentation.
   */
  vector_print_policy(const std::string& delim = "", unsigned int padding = 1)
    : vector_print_policy(delim, padding, "", 0, "", 0)
  {}

  /**
   * Return string delimiter used to separate values.
   */
  const std::string& delim() const { return delim_; }

  /**
   * Return number of spaces trailing delimiter before next value is printed.
   */
  unsigned int padding() const { return padding_; }

  /**
   * Return token printed before the first vector value is printed.
   */
  const std::string& pre_token() const { return pre_token_; }

  /**
   * Return number of spaces printed after `pre_token` before first value.
   */
  unsigned int pre_padding() const { return pre_padding_; }

  /**
   * Return token printed after the last vector value is printed.
   */
  const std::string& post_token() const { return post_token_; }

  /**
   * Return number of spaces printed before `post_token` after last value.
   */
  unsigned int post_padding() const { return post_padding_; }

private:
  std::string delim_;
  unsigned int padding_;
  std::string pre_token_;
  unsigned int pre_padding_;
  std::string post_token_;
  unsigned int post_padding_;
};

/**
 * Print a `std::vector` to `std::cout`.
 *
 * Useful for identifying the program in `examples` being run by the user. If
 * `T` is a user-defined type, make sure `operator<<` has been overloaded.
 *
 * @tparam T vector element type
 *
 * @param vec `const std::vector<T>&` vector to print out
 * @param print_policy `const vector_print_policy&` print policy
 * @param print `bool` where the vector is printed with an extra `\n` if `true`
 * @returns `std::string` giving the string to print
 */
template <typename T>
inline std::string print_vector(
  const std::vector<T>& vec,
  const vector_print_policy& print_policy,
  bool print = true)
{
  std::stringstream stream;
  stream << print_policy.pre_token();
  stream << std::string(print_policy.pre_padding(), ' ');
  for (const auto& v : vec) {
    stream << v;
    if (v != vec.back()) {
      stream << print_policy.delim() + std::string(print_policy.padding(), ' ');
    }
  }
  stream << std::string(print_policy.post_padding(), ' ');
  stream << print_policy.post_token();
  std::string vec_str = stream.str();
  if (print) {
    std::cout << vec_str << std::endl;
  }
  return vec_str;
}

/**
 * Print a `std::vector` to `std::cout` with default `vector_print_policy`.
 *
 * If `T` is a user-defined type, make sure `operator<<` has been overloaded.
 *
 * @tparam T vector element type
 *
 * @param vec `const std::vector<T>&` vector to print out
 * @param print `bool` where the vector is printed with an extra `\n` if `true`
 * @returns `std::string` giving the string to print
 */
template <typename T>
inline std::string print_vector(const std::vector<T>& vec, bool print = true)
{
  return print_vector(vec, vector_print_policy(), print);
}

/**
 * Create a new Boost vector from a `std::vector`.
 *
 * @tparam T `boost_vector` element type
 *
 * @param from `const std::vector<T>&` vector whose elements we will copy
 */
template <typename T>
inline boost_vector<T> boost_vector_from(const std::vector<T>& from)
{
  boost_vector<T> to(from.size());
  auto from_itr = from.begin();
  auto to_itr = to.begin();
  // note: when compiling with MSVC, using indexing causes C5045 to be emitted
  // with /Wall specified. but with iterators, no warning issued.
  while (from_itr != from.end() && to_itr != to.end()) {
    *to_itr = *from_itr;
    from_itr++;
    to_itr++;
  }
  return to;
}

/**
 * Create a new Boost vector with type `T` from a list of arguments.
 *
 * Uses a `static_cast<T>` to convert the arguments.
 *
 * Since the template `T` argument cannot be deduced, the recommended usage is:
 *
 * `auto bvec = boost_vector_from<T>(arg1, ... argN)`
 *
 * @tparam T `boost_vector` element type
 * @tparam Ts parameter pack
 *
 * @param from `Ts...` list of arguments that can be cast to `T`
 */
template <typename T, typename... Ts>
inline boost_vector<T> boost_vector_from(Ts... from)
{
  return boost_vector_from(std::vector<T>({static_cast<T>(from)...}));
}

}  // namespace pdmath

#endif  // PDMATH_HELPERS_H_
