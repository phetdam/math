/**
 * @file qmc_pi.cc
 * @author Derek Huang
 * @brief C++ program estimating pi using quasi Monte Carlo
 * @copyright MIT License
 */

#include <cstdlib>
#include <iostream>

#include "pdmath/pi.h"

int main()
{
  // number of [0, 1] partitions to make
  constexpr auto n = 5000u;
  // estimate using n * n samples
  std::cout << "pi (n_points=" << n * n << "): " << pdmath::qmc_pi(n) << std::endl;
  return EXIT_SUCCESS;
}
