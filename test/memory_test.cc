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
// of elements to allocate using the allocator
constexpr auto aligned_alloc_test_inputs = std::make_tuple(
  // note: posix_memalign() expects alignment >= sizeof(void*)
  std::tuple{pdmath::aligned_allocator<float, 8>{}, 47u},
  std::tuple{pdmath::aligned_allocator<int, 16>{}, 30u},
  std::tuple{pdmath::aligned_allocator<double, 16>{}, 55u}
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
   * Allocate aligned memory + check its alignment.
   */
  void operator()() const
  {
    // get allocator + allocation amount
    auto [alloc, n_elem] = std::get<I>(aligned_alloc_test_inputs);
    // value + deleter type
    using T = typename std::allocator_traits<decltype(alloc)>::value_type;
    using D = deallocator<decltype(alloc)>;
    // allocate + manage memory
    // note: CTAD is explicitly disallowed for this signature
    std::unique_ptr<T[], D> ptr{alloc.allocate(n_elem), D{alloc}};
    // ensure base address is appropriately aligned
    // note: reinterpret_cast<std::uintptr_t> well defined for pointers to
    // objects or cv-qualified void since C++11 if std::uintptr_t available
    EXPECT_EQ(0, reinterpret_cast<std::uintptr_t>(ptr.get()) % alloc.alignment());
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
