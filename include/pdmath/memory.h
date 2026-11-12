/**
 * @file memory.h
 * @author Derek Huang
 * @brief C++ header for dynamic memory management helpers
 * @copyright MIT License
 */

#ifndef PDMATH_MEMORY_H_
#define PDMATH_MEMORY_H_

#include <cstddef>
#include <cstdlib>
#include <new>

namespace pdmath {

/**
 * Alignment-aware allocator.
 *
 * This uses `aligned_alloc()` to allocate memory aligned on a certain multiple
 * of `sizeof(T)`. The primary use-case is for over-aligned types such as the
 * built-in SIMD types, e.g. ensuring alignment on a 32-byte boundary when
 * using `_mm256_store_pd()` with 4 packed doubles.
 *
 * This type satisfies the *Allocator* named requirements.
 *
 * @tparam T Value type
 */
template <typename T>
class aligned_allocator {
public:
  using value_type = T;

  /**
   * Allocate enough memory for the given number of instances of `T`.
   *
   * Memory aligned to `sizeof(T)` is allocated using `aligned_alloc()`.
   *
   * @param n Number of elements
   */
  T* allocate(std::size_t n)
  {
    auto p = std::aligned_alloc(sizeof(T), sizeof(T) * n);
    // TODO: include compile-time type name in message
    if (!p)
      throw std::bad_alloc{"unable to allocate aligned memory"};
    return p;
  }

  /**
   * Deallocate memory for the given number of instances of `T`.
   *
   * The number of elements passed to `allocate()` is ignored but it is of
   * course undefined behavior to call `deallocate()` on a chunk of memory that
   * was not originally allocated using `allocate()`.
   *
   * @param p Pointer to storage
   */
  void deallocate(T* p, std::size_t /*n*/) noexcept
  {
    std::free(p);
  }

  /**
   * Compare against another `aligned_allocator`.
   *
   * Since instances are stateless `true` is always returned.
   *
   * @param other `aligned_allocator` instance
   */
  constexpr bool operator==(const aligned_allocator& other) const noexcept
  {
    return true;
  }
};

}  // namespace pdmath

#endif  // PDMATH_MEMORY_H_
