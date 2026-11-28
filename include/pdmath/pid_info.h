/**
 * @file pid_info.h
 * @author Derek Huang
 * @brief C++ header for cross-platform PID info helpers
 * @copyright MIT License
 */

#ifndef PDMATH_PID_INFO_H_
#define PDMATH_PID_INFO_H_

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <errhandlingapi.h>
#include <libloaderapi.h>
#else
#include <unistd.h>
#endif  // !defined(_WIN32)

#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>

#ifdef _WIN32
#include "pdmath/warnings.h"
#endif  // _WIN32

namespace pdmath {

/**
 * Return the path to the currently executing process.
 */
inline std::filesystem::path pid_path()
{
#if defined(_WIN32)
  // buffer for GetModuleFileNameW()
  // note: MAX_PATH is 260 but we allow use with long paths
  std::wstring buf(MAX_PATH, L'\0');
  // call GetModuleFileNameW() until path isn't truncated
  for (
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4267)
    auto buf_len = GetModuleFileNameW(nullptr, buf.data(), buf.size());
PDMATH_MSVC_WARNINGS_POP()
    // 0 on error, truncation if buf_len == buf.size()
    // note: null-termination or not (Windows XP) doesn't matter since the
    // std::wstring buffer is null-terminated since C++11
    !buf_len || buf_len == buf.size();
    // if no error, buf_len == buf.size(), so truncation occurred
    buf = std::wstring(2 * buf.size(), L'\0')
  )
    if (!buf_len)
      throw std::system_error{
        static_cast<int>(GetLastError()), std::system_category(),
        "GetModuleFileNameW()"
      };
#else
  // buffer for readlink()
  std::string buf(256u, '\0');
  // call readlink() until path isn't truncated
  for (
    auto buf_len = readlink("/proc/self/exe", buf.data(), buf.size());
    // note: -1 on error so guaranteed to be greater than buf.size() and if
    // equal to buf.size() then truncation of the path may have occurred
    static_cast<std::size_t>(buf_len) >= buf.size();
    // if no error, buf_len == buf.size(), so truncation may have occurred
    buf = std::string(2 * buf.size(), '\0')
  )
    if (buf_len < 0)
      throw std::system_error{errno, std::system_category(), "readlink()"};
#endif  // !defined(_WIN32)
  // return path from c_str()
  // note: buf is filled with '\0' so guaranteed to be null-terminated
  return buf.c_str();
}

}  // namespace pdmath

#endif  // PDMATH_PID_INFO_H_
