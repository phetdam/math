/**
 * @file fib50.cc
 * @author Derek Huang
 * @brief C++ program that prints the first 50 Fibonacci numbers
 * @copyright MIT License
 *
 * The definition of Fibonacci numbers we use starts with F(0) = 0 to match the
 * OEIS list of the first 2000 Fibonacci numbers. So Fibonacci number 50 is
 * F(49), or 7778742049, which requires more than 32 bits to store.
 */

#include <cstdlib>
#include <iostream>

#include "pdmath/fibonacci.h"

int main()
{
  constexpr auto n = 50u;
  std::cout << pdmath::fibonacci_sequence<n - 1u>{} << std::endl;
  return EXIT_SUCCESS;
}
