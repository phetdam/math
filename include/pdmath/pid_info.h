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
#include <processthreadsapi.h>
#include <WinBase.h>
#else
#include <sys/types.h>
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
 * PID type.
 *
 * On Linux this must be signed to accommodate error codes.
 */
#if defined(_WIN32)
using pid_t = DWORD;
#else
using pid_t = ::pid_t;
#endif  // !defined(_WIN32)

#ifndef _WIN32
namespace detail {

/**
 * Return a `std::string` containing the contents of the symbolic link.
 *
 * This is used to implement `pid_path()` for Linux and grows the required
 * memory by a power of 2 each time. The buffer is guaranteed to be
 * null-terminated due to the use of `std::string` as the container.
 *
 * @param path Path to read from
 */
inline auto readlink(const char* path)
{
  // buffer for readlink()
  std::string buf(256u, '\0');
  // call readlink() until path isn't truncated
  for (
    auto buf_len = ::readlink(path, buf.data(), buf.size());
    // note: -1 on error so guaranteed to be greater than buf.size() and if
    // equal to buf.size() then truncation of the path may have occurred
    static_cast<std::size_t>(buf_len) >= buf.size();
    // if no error, buf_len == buf.size(), so truncation may have occurred
    buf = std::string(2 * buf.size(), '\0')
  )
    if (buf_len < 0)
      throw std::system_error{errno, std::system_category(), "readlink()"};
  return buf;
}

}  // namespace detail
#endif  // _WIN32

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
  // get buffer from readlink()
  auto buf = detail::readlink("/proc/self/exe");
#endif  // !defined(_WIN32)
  // return path from c_str()
  // note: buf is filled with '\0' so guaranteed to be null-terminated
  return buf.c_str();
}

/**
 * Return the path to the process identified by the given PID.
 *
 * @note Prefer the no-argument overload for `pid_path()` if the given PID will
 *  be that of the currently executing process.
 *
 * @param pid Process ID
 */
inline std::filesystem::path pid_path(pid_t pid)
{
#if defined(_WIN32)
  // scoping helper for HANDLE
  class unique_handle {
  public:
    unique_handle(HANDLE h) noexcept : h_{h} {}
    unique_handle(const unique_handle&) = delete;
    ~unique_handle() { CloseHandle(h_); }
    operator HANDLE() const noexcept { return h_; }
  private:
    HANDLE h_;
  };
  // get a handle to the specified process + check errors
  unique_handle ph = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (!ph)
    throw std::system_error{
      static_cast<int>(GetLastError()), std::system_category(),
      "OpenProcess()"
    };
  // buffer + length for QueryFullProcessImageNameW()
  // note: MAX_PATH is 260 but we allow use with long paths
  std::wstring buf(MAX_PATH, L'\0');
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4267)
  DWORD buf_len = buf.size();
PDMATH_MSVC_WARNINGS_POP()
  // call QueryFullProcessImageNameW() until path isn't truncated
  // note: this is simpler to use than GetModuleFileNameExW(). we cannot use
  // GetModuleFileNameW() since it only works for the current process's modules
  while (!QueryFullProcessImageNameW(ph, 0, buf.data(), &buf_len)) {
    switch (GetLastError()) {
    // continue if buffer is too small
    case ERROR_INSUFFICIENT_BUFFER:
    {
      buf_len *= 2u;
      buf = std::wstring(buf_len, L'\0');
      continue;
    }
    // other errors
    default:
      throw std::system_error{
        static_cast<int>(GetLastError()), std::system_category(),
        "QueryFullProcessImageNameW()"
      };
    }
  }
#else
  // get buffer from readlink("/proc/[pid]/exe")
  auto path = "/proc/" + std::to_string(pid) + "/exe";
  auto buf = detail::readlink(path.c_str());
#endif  // !defined(_WIN32)
  // return path from c_str()
  // note: buf is filled with '\0' so guaranteed to be null-terminated
  return buf.c_str();
}

}  // namespace pdmath

#endif  // PDMATH_PID_INFO_H_
