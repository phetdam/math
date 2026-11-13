/**
 * @file memory_test.cc
 * @author Derek Huang
 * @brief memory.h unit tests
 * @copyright MIT License
 */

#include "pdmath/memory.h"

#include <cstddef>
#include <memory>
#include <tuple>

#include <gtest/gtest.h>

#include "pdmath/common.h"
#include "pdmath/testing/utils.h"

namespace {

/**
 * Test class template for `memory.h` unit tests.
 *
 * @tparam T type
 */
template <typename T = void>
class MemoryTest : public ::testing::Test {};

/**
 * Test input type for aligned allocation tests.
 *
 * @tparam I Input index
 */
template <std::size_t I>
struct aligned_alloc_test {};

// aligned_alloc_test inputs. each is a tuple of the aligned allocator + number
// of elements to allocate using the allocator instance
constexpr auto aligned_alloc_test_inputs = std::make_tuple(
  std::tuple{pdmath::aligned_allocator<float, 8>{}, 47u},
  std::tuple{pdmath::aligned_allocator<int, 16>{}, 30u},
  std::tuple{pdmath::aligned_allocator<double, 16>{}, 55u},
  std::tuple{pdmath::aligned_allocator<float, 64>{}, 127u},
  // note: skipped for POSIX systems because posix_memalign() expects the
  // alignment to also be a power of sizeof(void*), which is alignof(void*)
  std::tuple{pdmath::aligned_allocator<double, 2>{}, 100u},
  std::tuple{pdmath::aligned_allocator<int, 128>{}, 200u}
);

/**
 * Partial specialization for `aligned_alloc_test<I>`.
 *
 * @tparam I Input index
 */
template <std::size_t I>
class MemoryTest<aligned_alloc_test<I>> : public ::testing::Test {
protected:
  static_assert(I < std::tuple_size_v<decltype(aligned_alloc_test_inputs)>);

  /**
   * Deleter type for an *Allocator*.
   *
   * This can be used as the deleter type for `std::unique_ptr`.
   *
   * @tparam A *Alloactor* type
   */
  template <typename A>
  class deallocator {
  public:
    using value_type = typename std::allocator_traits<A>::value_type;

    /**
     * Ctor.
     *
     * @param alloc Host allocator
     */
    deallocator(A& alloc) noexcept : alloc_{&alloc} {}

    /**
     * Invoke `deallocate()` on the pointer.
     *
     * This assumes that `allocate()` was never used to allocate zero bytes and
     * that `deallocate()` is not sensitive to its second parameter.
     *
     * @param p Pointer to memory
     */
    void operator()(value_type* p) const noexcept
    {
      alloc_->deallocate(p, 1u);
    }

  private:
    A* alloc_;
  };

  /**
   * Helper to indicate if compiling on Windows or not.
   *
   * We need this to conditionally enable test inputs with power of 2 memory
   * alignment that are *not* a power of `alignof(void*)`, which is required by
   * `posix_memalign()`, but not required by `_aligned_malloc()`.
   */
#if defined(_WIN32)
  static constexpr bool win32 = true;
#else
  static constexpr bool win32 = false;
#endif  // !defined(_WIN32)

  /**
   * Allocate aligned memory + check its alignment.
   */
  void operator()() const
  {
    // get allocator + allocation amount
    auto [alloc, n_elem] = std::get<I>(aligned_alloc_test_inputs);
    // get alignment at compile time
    constexpr auto alignment = decltype(alloc)::alignment();
    // if not on Windows and alignment not a power of sizeof(void*) as required
    // by posix_memalign(), then skip the test (would get an error anyways)
    if constexpr (!win32 && (alignment % sizeof(void*)))
      GTEST_SKIP() <<
        "alignment " << alignment << " is not a power of " << sizeof(void*) <<
        " as required by posix_memalign()";
    // otherwise run test
    else {
      // value + deleter type
      using T = typename std::allocator_traits<decltype(alloc)>::value_type;
      using D = deallocator<decltype(alloc)>;
      // allocate + manage memory
      // note: CTAD is explicitly disallowed for this signature
      std::unique_ptr<T[], D> ptr{alloc.allocate(n_elem), D{alloc}};
      // ensure base address is appropriately aligned
      // note: reinterpret_cast<std::uintptr_t> well defined for pointers to
      // objects or cv-qualified void since C++11 if std::uintptr_t available
      EXPECT_EQ(0, reinterpret_cast<std::uintptr_t>(ptr.get()) % alignment);
    }
  }
};

// input types
TYPED_TEST_SUITE(
  MemoryTest,
  PDMATH_IDENTITY(
    pdmath::testing::merge_test_types_t<
      pdmath::testing::indexed_types<
        aligned_alloc_test,
        std::tuple_size_v<decltype(aligned_alloc_test_inputs)>
      >
    >
  )
);

/**
 * Invoke the test.
 */
TYPED_TEST(MemoryTest, Test)
{
  (*this)();
}

}  // namespace
