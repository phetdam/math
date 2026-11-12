/**
 * @file memory.h
 * @author Derek Huang
 * @brief C++ header for dynamic memory management helpers
 * @copyright MIT License
 */

#ifndef PDMATH_MEMORY_H_
#define PDMATH_MEMORY_H_

#if defined(_WIN32)
#include <malloc.h>          // _aligned_alloc(), aligned_free()
#else
#include <stdlib.h>          // posix_memalign()
#endif  // !defined(_WIN32)

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <new>

namespace pdmath {

/**
 * Alignment-aware allocator.
 *
 * This allocator is capable of allocating a chunk of memory with the starting
 * address being a certain multiple of alignment `A`. The primary use-case is
 * for over-aligned allocation for interaction with types like the built-in
 * SIMD types, e.g. ensuring alignment on a 32-byte boundary when using
 * `_mm256_store_pd()` with a buffer provided by this allocator.
 *
 * This type satisfies the *Allocator* named requirements.
 *
 * @tparam T Value type
 * @tparam A Memory alignment in bytes
 */
template <typename T, std::size_t A = alignof(std::max_align_t)>
class aligned_allocator {
public:
  using value_type = T;

  /**
   * Allocate enough memory for the given number of instances of `T`.
   *
   * Memory aligned to `A` is allocated using `_aligned_alloc()` on Windows and
   * using `posix_memalign()` for POSIX systems. We do not use `aligned_alloc()`
   * because the alignment requirement only applies to the lowest address in
   * the buffer, *not* to every single element we wish to allocate.
   *
   * For example, if a `float` buffer was allocated using the specialization
   * `aligned_allocator<float, 32>`, the buffer would be placed at an address
   * that is a multiple of 32. Using unaligned offsets from the base address of
   * the chunk with `_mm256_store_pd()`, however, would result in unaligned
   * memory access and could result in a general-protection fault.
   *
   * @param n Number of elements
   */
  T* allocate(std::size_t n)
  {
#if defined(_WIN32)
    auto p = _aligned_alloc(sizeof(T) * n, A);
    if (!p)
      throw std::bad_alloc{"_aligned_alloc(): unable to allocate aligned memory"};
#else
    void *p;
    switch (posix_memalign(&p, A, sizeof(T) * n)) {
    case EINVAL:
      throw std::bad_alloc{"posix_memalign(): invalid parameter"};
    case ENOMEM:
      throw std::bad_alloc{"posix_memalign(): out of memory"};
    default:
      return p;
    }
#endif  // !defined(_WIN32)
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
#if defined(_WIN32)
    _aligned_free(p);
#else
    std::free(p);
#endif  // !defined(_WIN32)
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
