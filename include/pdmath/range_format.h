/**
 * @file range_format.h
 * @author Derek Huang
 * @brief C++ header for range output formatting
 * @copyright MIT License
 */

#include <iostream>  // TODO: remove when print_vector is removed
#include <ostream>
#include <string>
#include <utility>

#include "pdmath/type_traits.h"
#include "pdmath/warnings.h"

#ifndef PDMATH_RANGE_FORMAT_H_
#define PDMATH_RANGE_FORMAT_H_

namespace pdmath {

/**
 * Control structure holding range print formatting options.
 *
 * This is a refactored version of the original that was located in `helpers.h`
 * that adopts more modern practices and no longer works directly with a
 * `print_vector` function but with `pdmath::ostream`.
 */
class range_format_policy {
public:
  /**
   * Ctor.
   *
   * @todo Use a fluent interface instead for less ambiguity.
   *
   * @param delim Token used to separate range values
   * @param padding Spaces to print after `delim` before the next value
   * @param pre_token Token printed before the values
   * @param pre_padding Spaces to print after `pre_token` before first value
   * @param post_token Token printed after the values
   * @param post_padding Spaces to print before `post_token` after last value
   */
  range_format_policy(
    std::string delim,
    unsigned padding,
    std::string pre_token,
    unsigned pre_padding,
    std::string post_token,
    unsigned post_padding)
    : delim_{std::move(delim)},
      padding_{padding},
      pre_token_{std::move(pre_token)},
      pre_padding_{pre_padding},
      post_token_{std::move(post_token)},
      post_padding_{post_padding}
  {}

// MSVC can't guarantee left-to-right evaluation in braced-init-list
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4868)
#endif  // _MSC_VER
  /**
   * Ctor.
   *
   * This is a simplified version with no pre/post padding or tokens.
   *
   * See the full constructor for parameter documentation.
   */
  range_format_policy(std::string delim = "", unsigned int padding = 1)
    : range_format_policy{std::move(delim), padding, "", 0, "", 0}
  {}
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER

  /**
   * Return string delimiter used to separate values.
   */
  auto& delim() const noexcept { return delim_; }

  /**
   * Return number of spaces trailing delimiter before next value is printed.
   */
  auto padding() const noexcept { return padding_; }

  /**
   * Return token printed before the first vector value is printed.
   */
  auto& pre_token() const noexcept { return pre_token_; }

  /**
   * Return number of spaces printed after `pre_token` before first value.
   */
  auto pre_padding() const noexcept { return pre_padding_; }

  /**
   * Return token printed after the last vector value is printed.
   */
  auto& post_token() const noexcept { return post_token_; }

  /**
   * Return number of spaces printed before `post_token` after last value.
   */
  auto post_padding() const noexcept { return post_padding_; }

private:
  std::string delim_;
  unsigned int padding_;
  std::string pre_token_;
  unsigned int pre_padding_;
  std::string post_token_;
  unsigned int post_padding_;
};

/**
 * Proxy formatting object for ranges.
 *
 * This holds a reference to the original `std::ostream` and the print policy
 * and is used to trigger a special `operator<<` overload for range-like types.
 *
 * For convenience, this type is trivially copyable.
 */
class range_format_proxy {
public:
  /**
   * Ctor.
   *
   * The caller is expected to ensure the referred objects are not destroyed
   * until the proxy object itself is destroyed.
   *
   * @param out Output stream
   * @param policy Range formatting policy
   */
  range_format_proxy(
    std::ostream& out,
    const range_format_policy& policy) noexcept : out_{&out}, policy_{&policy} {}

  /**
   * Return a reference to the underlying output stream.
   */
  auto& out() noexcept { return *out_; }

  /**
   * Return a const reference to the borrowed range formatting policy.
   */
  auto& policy() const noexcept { return *policy_; }

private:
  std::ostream* out_;
  const range_format_policy* policy_;
};

/**
 * Return the range formatting proxy object.
 *
 * This function assumes that the lifetime of the policy persists after the
 * function returns, i.e. that the reference in the proxy object is valid. This
 * will be true in most cases, even when using a prvalue `policy` in an
 * expression, due to the fact that temporary objects live until the end of a
 * full-expression. Therefore, expressions like this are fine:
 *
 * @code{.cc}
 * std::vector vec{1., 2., 3.};
 * std::cout << range_format_policy{} << vec << std::endl;
 * @endcode
 *
 * However, assigning the proxy object to an lvalue will be an issue as the
 * full-expression is the overall assignment:
 *
 * @code{.cc}
 * std::vector vec{1., 2., 3.};
 * auto proxy = std::cout << range_format_policy{};  // policy destroyed
 * proxy << vec << std::endl;  // dangling reference to policy accessed
 * @endcode
 *
 * @param out Output stream
 * @param policy Range formatting policy
 */
inline
auto operator<<(std::ostream& out, const range_format_policy& policy)
{
  return range_format_proxy{out, policy};
}

/**
 * Return a new range formatting proxy object.
 *
 * This overload is not very useful but allows one to provide a new range
 * format policy in lieu of the existing one used the range format proxy.
 * There generally should be no need to use this function but it is provided
 * so `std::cout << policy_1 << policy_2 << my_range` works correctly.
 *
 * As usual, the underlying output stream should still be within its lifetime
 * and the new proxy object should not be destroyed after function return.
 *
 * @param proxy Proxy object with underlying `std::ostream`
 * @param policy Range formatting policy
 */
inline
auto operator<<(range_format_proxy proxy, const range_format_policy& policy)
{
  return range_format_proxy{proxy.out(), policy};
}

/**
 * Write a range to the output stream using the formatting policy.
 *
 * A reference to the underlying output stream is returned to resume "normal"
 * `std::ostream` formatting. The policy needs to be specified again if another
 * range is to be formatted in the same manner.
 *
 * @note See `operator<<(std::ostream&, const range_format_policy&)` for
 *  details on temporary object lifetime to prevent dangling.
 *
 * @note The range is assumed to provide iterators that satisfy the
 *  *LegacyInputIterator* named requirements that yield streamable values.
 *
 * @param proxy Proxy object with underlying `std::ostream`
 * @param range Range to write to output stream
 */
template <typename R, constraint_t<is_range_v<R>> = 0>
auto& operator<<(range_format_proxy proxy, R&& range)
{
  // convenience references
  auto& out = proxy.out();
  const auto& policy = proxy.policy();
  // print pre-token and pre-padding
  out << policy.pre_token() << std::string(policy.pre_padding(), ' ');
  // get iterators + flag for printing delims
  auto it = std::begin(range);
  auto end = std::end(range);
  bool print_delim = false;
  // print values
  while (it != end) {
    if (print_delim)
      out << policy.delim() << std::string(policy.padding(), ' ');
    out << *it++;
    // ensure that subsequent elements have delimiter printed
    print_delim = true;
  }
  // print post-padding and post-token + return
  return out << std::string(policy.post_padding(), ' ') << policy.post_token();
}

/**
 * Write a non-range value to the output stream.
 *
 * This simply delegates to `operator<<` for `std::ostream` if any.
 *
 * @param proxy Proxy object with underlying `std::ostraem`
 * @param value Non-range value to write
 */
template <typename T, constraint_t<!is_range_v<T>> = 0>
auto& operator<<(range_format_proxy proxy, T&& value)
{
  return proxy.out() << value;
}

/**
 * Format the given range as a string.
 *
 * @tparam R Range to format
 *
 * @param range Range to format
 * @param policy Range formatting policy
 */
template <typename R>
auto to_string(
  R&& range,
  const range_format_policy& policy = {},
  constraint_t<is_range_v<R>> = 0)
{
  std::stringstream ss;
  ss << policy << range;
  return ss.str();
}

// TODO: remove print_vector compatibility function templates later

/**
 * Print elements of a range to `std::cout`.
 *
 * It is assumed that the range's value type is streamable to `std::ostream`.
 *
 * @tparam T Range to print
 *
 * @param vec Range to print
 * @param policy Range formatting policy
 * @param print `true` to write the range to `std::cout` with an extra `\n`
 * @returns `std::string` giving the string to print
 */
template <typename T>
auto print_vector(
  const T& vec,
  const range_format_policy& policy,
  bool print = true,
  constraint_t<is_range_v<T>> = 0)
{
  // first obtain formatted string
  auto str = to_string(vec, policy);
  // print to std::cout if requested + return
  if (print)
    std::cout << str << std::endl;
  return str;
}

/**
 * Print elements of a range to `std::cout` using default formatting policy.
 *
 * It is assumed that the range's value type is streamable to `std::ostream`.
 *
 * @tparam T Range to print
 *
 * @param vec Range to print
 * @param print `true` to write the range to `std::cout` with an extra `\n`
 * @returns `std::string` giving the string to print
 */
template <typename T>
auto print_vector(
  const T& vec,
  bool print = true,
  constraint_t<is_range_v<T>> = 0)
{
  // note: use explicit type since constraint adds ambiguity
  return print_vector(vec, range_format_policy{}, print);
}

}  // namespace pdmath

#endif  // PDMATH_RANGE_FORMAT_H_
