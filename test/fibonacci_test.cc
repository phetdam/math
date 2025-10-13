/**
 * @file fibonacci_test.cc
 * @author Derek Huang
 * @brief fibonacci.h unit tests
 * @copyright MIT License
 */

#include "pdmath/fibonacci.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "pdmath/warnings.h"

namespace {

/**
 * Test fixture for Fibonacci sequence tests.
 */
class FibonacciTest : public ::testing::Test {
protected:
  // number of Fibonacci numbers
  static constexpr auto n_fib_ = 50u;
// MSVC emits 5246 erroneously even in C++17 mode
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5246)
  // n_fib_ numbers from OEIS list https://oeis.org/A000045/b000045.txt
  static constexpr std::array<std::uint_fast64_t, n_fib_> expected_ = {
    0,
    1,
    1,
    2,
    3,
    5,
    8,
    13,
    21,
    34,
    55,
    89,
    144,
    233,
    377,
    610,
    987,
    1597,
    2584,
    4181,
    6765,
    10946,
    17711,
    28657,
    46368,
    75025,
    121393,
    196418,
    317811,
    514229,
    832040,
    1346269,
    2178309,
    3524578,
    5702887,
    9227465,
    14930352,
    24157817,
    39088169,
    63245986,
    102334155,
    165580141,
    267914296,
    433494437,
    701408733,
    1134903170,
    1836311903,
    2971215073,
    4807526976,
    7778742049
  };
PDMATH_MSVC_WARNINGS_POP()
  // retrieve matcher for elementwise comparison
  // note: can't use ContainerEq since iterator types would differ
  auto AllEq() const
  {
    return ::testing::Pointwise(::testing::Eq(), expected_);
  }
};

/**
 * Test that the Fibonacci generator produces the correct values.
 */
TEST_F(FibonacciTest, GeneratorTest)
{
  // Fibonacci generator
  pdmath::fibonacci_generator gen;
  // generate n_fib_ sequence into vector
  std::vector<std::uint_fast64_t> seq(n_fib_);
  std::for_each(seq.begin(), seq.end(), [&gen](auto& v) { v = *gen++; });
  // compare elementwise for equality
  EXPECT_THAT(seq, AllEq());
}

/**
 * Test that the compile-time Fibonacci sequence produces the correct values.
 */
TEST_F(FibonacciTest, TemplateTest)
{
  EXPECT_THAT(pdmath::fibonacci_sequence<n_fib_ - 1u>::value, AllEq());
}

}  // namespace
