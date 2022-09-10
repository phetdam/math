/**
 * @file helpers.h
 * @author Derek Huang
 * @brief C++ header for project-specific helper macros
 * @copyright MIT License
 */

#ifndef PDMATH_HELPERS_H_
#define PDMATH_HELPERS_H_

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <Eigen/Core>

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
 * Print elements of a *Container* to `std::cout`.
 *
 * Useful for identifying the program in `examples` being run by the user. If
 * `V_t::value_type` is user-defined, it must overload `operator<<`.
 *
 * @tparam V_t *Container* to print
 *
 * @param vec `const V_t&` *Container* to print out
 * @param print_policy `const vector_print_policy&` print policy
 * @param print `bool` where `vec` is printed with an extra `\n` if `true`
 * @returns `std::string` giving the string to print
 */
template <typename V_t>
std::string print_vector(
  const V_t& vec,
  const vector_print_policy& print_policy,
  bool print = true)
{
  std::stringstream stream;
  stream << print_policy.pre_token();
  stream << std::string(print_policy.pre_padding(), ' ');
  for (const auto& v : vec) {
    if (v != *vec.begin()) {
      stream << print_policy.delim() + std::string(print_policy.padding(), ' ');
    }
    stream << v;
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
 * Print a *Container* to `std::cout` with default `vector_print_policy`.
 *
 * If `V_t::value_type` is user-defined, it must overload `operator<<`.
 *
 * @tparam V_t *Container* to print
 *
 * @param vec `const V_t&` *Container* to print out
 * @param print `bool` where `vec` is printed with an extra `\n` if `true`
 * @returns `std::string` giving the string to print
 */
template <typename V_t>
inline std::string print_vector(const V_t& vec, bool print = true)
{
  return print_vector(vec, vector_print_policy(), print);
}

/**
 * Create a new Boost vector from a *Container*.
 *
 * @tparam V_t *Container* to copy elements from
 *
 * @param from `const V_t&` *Container* whose elements we will copy
 */
template <typename V_t>
inline boost_vector<typename V_t::value_type> boost_vector_from(const V_t& from)
{
// MSVC complains about signed to unsigned conversion if using Eigen vectors
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4365)
#endif  // _MSC_VER
  boost_vector<typename V_t::value_type> to(from.size());
#ifdef _MSC_VER
#pragma warning (pop)
#endif  // _MSC_VER
  // note: when compiling with MSVC, using indexing when copying using a for
  // loop causes C5045 to be emitted with /Wall, but not with iterators.
  std::copy(from.cbegin(), from.cend(), to.begin());
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

/**
 * Create a new Eigen dynamic vector from a `std::vector<T>`.
 *
 * @tparam T new vector element type
 *
 * @param from `const std::vector<T>&` vector whose elements we will copy
 */
template <typename T>
inline Eigen::VectorX<T> eigen_vector_from(const std::vector<T>& from)
{
  Eigen::VectorX<T> to(from.size());
  std::copy(from.cbegin(), from.cend(), to.begin());
  return to;
}

/**
 * Create a new Eigen dynamic vector with type `T` from a list of arguments.
 *
 * Uses a `static_cast<T>` to convert the arguments.
 *
 * Since the template `T` argument cannot be deduced, the recommended usage is:
 *
 * `auto evec = eigen_vector_from<T>(arg1, ... argN);`
 *
 * @tparam T `Eigen::VectorX` element type
 * @tparam Ts parameter pack
 *
 * @param from `const Ts&...` list of arguments that can be cast to `T`
 */
template <typename T, typename... Ts>
inline auto eigen_vector_from(const Ts&... from)
{
  return Eigen::VectorX<T>{{static_cast<T>(from)...}};
}

/**
 * Create a new Eigen dynamic vector from any `Eigen::MatrixBase`.
 *
 * 2D matrix elements will be read in expected row-major order.
 *
 * @tparam Derived The derived matrix/matrix expression type
 *
 * @param from `const Eigen::MatrixBase<T>&` to create new vector from
 */
template <typename Derived>
inline Eigen::VectorX<typename Derived::Scalar> eigen_vector_from(
  const Eigen::MatrixBase<Derived>& from)
{
  Eigen::VectorX<typename Derived::Scalar> to(from.size());
  // reshape returns a view, not actually moving elements around
  auto flat_view = from.transpose().reshaped();
  std::copy(flat_view.cbegin(), flat_view.cend(), to.begin());
  return to;
}

/**
 * Create a new `V_t` *Container* from a list of arguments.
 *
 * Since the template `V_t` argument cannot be deduced, recommended usage is:
 *
 * `auto evec = vector_from<V_t>(arg1, ... argN);`
 *
 * @tparam V_t *Container* type list-initializable by initializer list
 * @tparam Ts parameter pack
 *
 * @param from `const Ts&...` list of arguments castable to `V_t::value_type`
 */
template <typename V_t, typename... Ts>
inline V_t vector_from(const Ts&... from)
{
  return V_t{{static_cast<typename V_t::value_type>(from)...}};
}

/**
 * Create a new `VOut_t` *Container* from another `VIn_t` *Container*.
 *
 * Since the template `V_t` argument cannot be deduced, recommended usage is:
 *
 * `auto evec = vector_from<VOut_t>(from);`
 *
 * @tparam VOut_t *Container* type list-initializable by initializer list
 * @tparam VIn_t *Container* type
 *
 * @param from `const VIn_t&` *Container* to copy elements from
 */
template <typename VOut_t, typename VIn_t>
inline VOut_t vector_from(const VIn_t& from)
{
// MSVC complains about signed to unsigned conversion if using Eigen vectors
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4365)
#endif  // _MSC_VER
  VOut_t to(from.size());
#ifdef _MSC_VER
#pragma warning (pop)
#endif // _MSC_VER
  std::copy(from.cbegin(), from.cend(), to.begin());
  return to;
}

/**
 * Create a new `std::vector` from a `V_t` *Container* with same element type.
 *
 * @tparam V_t *Container* type
 *
 * @param from `const VIn_t&` *Container* to copy elements from
 */
template <typename V_t>
inline std::vector<typename V_t::value_type> vector_from(const V_t& from)
{
  return vector_from<std::vector<typename V_t::value_type>>(from);
}

// TODO: document unique_vector_from + add generic VIn_t, VOut_t overload
template <typename V_t, typename... Ts>
inline std::unique_ptr<V_t> unique_vector_from(const Ts&... from)
{
  return std::unique_ptr<V_t>(
    new V_t{{static_cast<typename V_t::value_type>(from)...}}
  );
}

template <typename VOut_t, typename VIn_t>
inline std::unique_ptr<VOut_t> unique_vector_from(const VIn_t& from)
{
  auto to = std::make_unique<VOut_t>(from.size());
  std::copy(from.cbegin(), from.cend(), to->begin());
  return to;
}

}  // namespace pdmath

#endif  // PDMATH_HELPERS_H_
