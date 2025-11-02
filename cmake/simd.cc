/**
 * @file simd.cc
 * @author Derek Huang
 * @brief C++ program to check supported CPU SIMD features
 * @copyright MIT License
 *
 * This will call `cpuid` to obtain CPU features on x86 systems. On ARM systems
 * the program will still compile but when run all feature flags are zero.
 * Feature flag identifiers are formatted for CMake's convenience.
 *
 * @note This is a work-in-progress because we aren't using all SIMD features.
 */

#include <cstdlib>
#include <iostream>

#include "pdmath/cpu_info.h"

int main()
{
  pdmath::cpu_info info;
  std::cout <<
    "MMX         " << info.mmx() << "\n" <<
    "SSE         " << info.sse() << "\n" <<
    "SSE2        " << info.sse2() << "\n" <<
    "SSE3        " << info.sse3() << "\n" <<
    "SSSE3       " << info.ssse3() << "\n" <<
    "SSE4_1      " << info.sse4_1() << "\n" <<
    "SSE4_2      " << info.sse4_2() << "\n" <<
    "AVX         " << info.avx() << "\n" <<
    "FMA         " << info.fma() << "\n" <<
    "AVX2        " << info.avx2() << "\n" <<
    "AVX512F     " << info.avx512f() << "\n" << std::flush;
  return EXIT_SUCCESS;
}
