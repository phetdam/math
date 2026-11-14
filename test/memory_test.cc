/**
 * @file memory_test.cc
 * @author Derek Huang
 * @brief memory.h unit tests
 * @copyright MIT License
 */

#include "pdmath/memory.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/common.h"
#include "pdmath/functional.h"
#include "pdmath/testing/utils.h"
#include "pdmath/type_traits.h"

namespace {

/**
 * Test class template for `memory.h` unit tests.
 *
 * @tparam T type
 */
template <typename T = void>
class MemoryTest : public ::testing::Test {};

/**
 * Base specialization for shared helpers.
 */
template <>
class MemoryTest<void> : public ::testing::Test {
public:
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
   * Check that an integral type is a power of 2.
   *
   * @tparam T Integral type
   *
   * @param v Integral value
   */
  template <typename T>
  static constexpr bool has_one_bit(
    T v,
    pdmath::constraint_t<std::is_integral_v<T>> = {}) noexcept
  {
    auto n_bits = 0u;
    while (v) {
      n_bits += (v & 0x1);
      v >>= 1;
    }
    return n_bits == 1u;
  }

  /**
   * Run the test only if the alignment is valid for `posix_memalign()`.
   *
   * This construct allows us to skip tests using `GTEST_SKIP()` when the given
   * alignment requirements are not a power of `sizeof(void*)` as required by
   * `posix_memalign()`. `_aligned_malloc()` only requires a power of 2 and the
   * extra check is therefore skipped when running on Windows.
   *
   * @tparam A Memory alignment
   * @tparam F Test invocable
   *
   * @param func Test invocable
   */
  template <std::size_t A, typename F>
  static void test(
    F&& func,
    pdmath::constraint_t<std::is_invocable_v<F>> = {})
  {
    // alignment must be a power of two
    static_assert(has_one_bit(A), "alignment must be power of 2");
    // if not on Windows and alignment not a power of sizeof(void*) as required
    // by posix_memalign(), then skip the test (would get an error anyways)
    if constexpr (!win32 && (A % sizeof(void*)))
      GTEST_SKIP() <<
        "alignment " << A << " is not a power of " << sizeof(void*) <<
        " as required by posix_memalign()";
    // otherwise run test
    else
      std::invoke(func);
  }
};

/**
 * Test input type for aligned allocation tests.
 *
 * @tparam I Input index
 */
template <std::size_t I>
struct aligned_alloc_test {};

// aligned_alloc_test inputs. each is a pair of the aligned allocator + number
// of elements to allocate using the allocator instance
constexpr auto aligned_alloc_test_inputs = std::make_tuple(
  std::pair{pdmath::aligned_allocator<float, 8>{}, 47u},
  std::pair{pdmath::aligned_allocator<int, 16>{}, 30u},
  std::pair{pdmath::aligned_allocator<double, 16>{}, 55u},
  std::pair{pdmath::aligned_allocator<float, 64>{}, 127u},
  // note: skipped for POSIX systems because posix_memalign() expects the
  // alignment to also be a multiple of sizeof(void*), which is alignof(void*)
  std::pair{pdmath::aligned_allocator<double, 2>{}, 100u},
  std::pair{pdmath::aligned_allocator<int, 32>{}, 200u}
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
   * @tparam A *Allocator* type
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
    // get allocator, allocation amount, and compile-time alignment
    auto [alloc, n_elem] = std::get<I>(aligned_alloc_test_inputs);
    constexpr auto alignment = decltype(alloc)::alignment();
    // run test
    MemoryTest<>::test<alignment>(
      [&alloc, n_elem]
      {
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
    );
  }
};

/**
 * Test input type for aligned vector tests.
 *
 * @tparam I Input index
 */
template <std::size_t I>
struct aligned_vector_test {};

// aligned_vector_test inputs. each specifies an allocator and the number of
// elements to allocate which will be initialized using iota()
constexpr auto aligned_vector_test_inputs = std::make_tuple(
  std::pair{pdmath::aligned_allocator<float, 32>{}, 55u},
  std::pair{pdmath::aligned_allocator<float, 4>{}, 64u},
  std::pair{pdmath::aligned_allocator<int, 4>{}, 128u},
  std::pair{pdmath::aligned_allocator<double, 64>{}, 100u},
  std::pair{pdmath::aligned_allocator<double, 16>{}, 240u}
);

/**
 * Partial specialization for `aligned_vector_test<I>`.
 *
 * @tparam I Input index
 */
template <std::size_t I>
class MemoryTest<aligned_vector_test<I>> : public ::testing::Test {
protected:
  void operator()() const
  {
    // get allocator, allocation amount, and compile-time alignment
    auto [alloc, n_elem] = std::get<I>(aligned_alloc_test_inputs);
    constexpr auto alignment = decltype(alloc)::alignment();
    // run test
    MemoryTest<>::test<alignment>(
      [n_elem]
      {
        // allocator value type
        using T = typename std::allocator_traits<decltype(alloc)>::value_type;
        // create standard + specific alignment vectors
        std::vector<T> v0(n_elem);
        std::vector<T, decltype(alloc)> v1(n_elem);
        // initialize
        pdmath::iota(v0);
        pdmath::iota(v1);
        // compare equal
        EXPECT_THAT(v1, ::testing::Pointwise(::testing::Eq(), v0));
      }
    );
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
      >,
      pdmath::testing::indexed_types<
        aligned_vector_test,
        std::tuple_size_v<decltype(aligned_vector_test_inputs)>
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
