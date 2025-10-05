/**
 * @file program_banner.h
 * @author Derek Huang
 * @brief C++ header for a program banner stream manipulator
 * @copyright MIT License
 */

#ifndef PDMATH_PROGRAM_BANNER_H_
#define PDMATH_PROGRAM_BANNER_H_

#include <ostream>
#include <string>
#include <string_view>

namespace pdmath {

/**
 * Program banner stream manipulator.
 *
 * For a given file name this is used to trigger `operator<<` overload
 * selection to write the name of a specified file inside an ASCII frame. For
 * example, given a path `/path/to/my_file.cc`, the banner would be
 *
 * ```
 * +---------+
 * | my_file |
 * +---------+
 * ```
 *
 * This type is trivially copyable and usable in constant expressions.
 * Furthermore, it works with both Windows and POSIX paths.
 */
class program_banner {
public:
  /**
   * Ctor.
   *
   * This determines the file name from the path-like string on construction.
   *
   * @param path Path-like string
   */
  constexpr program_banner(std::string_view path) noexcept
    : path_{path}
  {
    // iterators
    auto begin = path_.begin();
    auto end = path_.end();
    auto it = --end;
    // move backwards until we hit the first path component
    while (it != begin && *it != '/' && *it != '\\')
      it--;
    // if we are sitting at a path separator, move forward to avoid it
    if (*it == '/' || *it == '\\')
      it++;
    // update begin iterator
    begin = it;
    it = end - 1;
    // now move backwards until first . is hit
    while (it != begin && *it != '.')
      it--;
    // if we are at begin, either no '.' in file name or file starts with '.',
    // in which case we leave it as-is (e.g. could be a dotfile)
    // note: before C++20 there is no {begin, end} ctor available yet. we also
    // need to cast to enable braced-list-init (otherwise narrowing error)
    if (it == begin)
      stem_ = {&*begin, static_cast<std::size_t>(end - begin)};
    else
      stem_ = {&*begin, static_cast<std::size_t>(it - begin)};
  }

  /**
   * Return the original path-like string view.
   */
  constexpr auto path() const noexcept { return path_; }

  /**
   * Return the stem of the path-like string.
   *
   * This will be missing the final extension in the path unless the path
   * itself is `.` or a dotfile like `.bashrc`.
   */
  constexpr auto stem() const noexcept { return stem_; }

private:
  std::string_view path_;
  std::string_view stem_;
};

/**
 * Define the program banner for the current file.
 */
#define PDMATH_PROGRAM_BANNER() ::pdmath::program_banner{__FILE__}

/**
 * Write the actual banner to an output stream.
 *
 * The streamed banner is terminated with a newline for convenience.
 *
 * @param out Output stream
 * @param banner Program banner object
 */
inline
auto& operator<<(std::ostream& out, const program_banner& banner)
{
  // file stem we will be writing
  auto stem = banner.stem();
  // frame of '-' chars. we add 2 for padding
  std::string frame(stem.size() + 2u, '-');
  // write to stream
  return out << "+" << frame << "+\n| " << stem << " |\n+" << frame << "+\n";
}

}  // namespace pdmath

#endif  // PDMATH_PROGRAM_BANNER_H_
