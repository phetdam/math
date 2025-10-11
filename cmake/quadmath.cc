/**
 * @file quadmath.cc
 * @author Derek Huang
 * @brief C++ program to check that libquadmath is available for GCC
 * @copyright MIT License
 */

#include <quadmath.h>

#include <cstdio>
#include <cstdlib>

int main()
{
  // format FLT128_MAX as a string. by using one of the extern symbols from
  // libquadmath, we can verify that the .so is also locatable by GCC
  char buf[64];
  quadmath_snprintf(buf, sizeof buf, "%Qf", FLT128_MAX);
  std::printf("FLT128_MAX: %s\n", buf);
  return EXIT_SUCCESS;
}
