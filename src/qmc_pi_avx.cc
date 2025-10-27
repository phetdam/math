/**
 * @file qmc_pi_avx.cc
 * @author Derek Huang
 * @brief C++ program estimating pi using AVX quasi Monte Carlo
 * @copyright MIT License
 */

#include <cstdlib>
#include <iostream>

#include "pdmath/impl_policy.h"
#include "pdmath/pi.h"
#include "pdmath/scoped_timer.h"

int main()
{
  // number of [0, 1] partitions to make
  constexpr auto n = 5000u;
  // compute time
  std::chrono::milliseconds time;
  // estimate using n * n samples
  auto pi_hat = [&time]
  {
    pdmath::scoped_timer timer{time};
    return pdmath::qmc_pi(pdmath::simd, n);  // explicitly select SIMD overload
  }();
  // print
  std::cout << "pi (n_points=" << n * n << "): " << pi_hat << " in " <<
    time.count() << " ms" << std::endl;
  return EXIT_SUCCESS;
}
