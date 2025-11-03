/**
 * @file demangle.h
 * @author Derek Huang
 * @brief C++ header for type name demangling
 * @copyright MIT License
 */

#ifndef PDMATH_DEMANGLE_H_
#define PDMATH_DEMANGLE_H_

#include "pdmath/features.h"  // for PDMATH_ITANIUM_ABI

#if PDMATH_ITANIUM_ABI
#include <cxxabi.h>
#endif  // PDMATH_ITANIUM_ABI

// other STL headers necessary for abi::__cxa_demangle
#if PDMATH_ITANIUM_ABI
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#endif  // PDMATH_ITANIUM_ABI
#include <string_view>
#include <typeinfo>  // for well-formed typeid() usage

namespace pdmath {

/**
 * Return a null-terminated view over a demangled C++ type name.
 *
 * For compilers adhering to the Itanium ABI `abi::__cxa_demangle` is used with
 * a thread-local buffer. Otherwise, the input name is returned as-is.
 *
 * This function will throw if `abi::__cxa_demangle` encounters an error.
 *
 * @param name Mangled null-terminated C++ type name
 */
inline std::string_view demangle(const char* name) noexcept(!PDMATH_ITANIUM_ABI)
{
// Itanium ABI compilers like GCC, Clang
#if PDMATH_ITANIUM_ABI
  // malloc deleter
  struct malloc_deleter {
    void operator()(void* buf) noexcept { std::free(buf); }
  };
  // thread local buffer for allocating the demangled name
  thread_local std::unique_ptr<char[], malloc_deleter> buf;
  // demangle
  int status;
  std::size_t length;
  buf = decltype(buf){abi::__cxa_demangle(name, nullptr, &length,  &status)};
  // check status
  switch (status) {
  // malloc() failed
  case -1:
    throw std::runtime_error{"unable to allocate memory for buffer"};
  // invalid Itanium ABI mangled name
  case -2:
    throw std::runtime_error{"type name not a valid Itanium ABI mangled name"};
  // invalid argument
  case -3:
    throw std::runtime_error{"__cxa_demangle received an invalid argument"};
  // ok, success
  default:
    return buf.get();
  }
// non-Itanium ABI compilers like MSVC
#else
  return name;
#endif  // !PDMATH_ITANIUM_ABI
}

/**
 * Return a null-terminated view over the demangled C++ type name.
 *
 * This overload is provided as a convenience when working with `typeid()`,
 *
 * See `demangle(const char*)` for details.
 *
 * @param info C++ type info
 */
inline auto demangle(const std::type_info& info) noexcept(!PDMATH_ITANIUM_ABI)
{
  return demangle(info.name());
}

/**
 * Obtain a null-terminated view over the expression's demangled C++ type name.
 *
 * This is a convenience macro for using `typeid()` on C++ expressions.
 *
 * @param expr Expression to get a type name for
 */
#define PDMATH_DEMANGLE(expr) ::pdmath::demangle(typeid(expr))

}  // namespace pdmath

#endif  // PDMATH_DEMANGLE_H_
