/**
 * @file cpu_info.cc
 * @author Derek Huang
 * @brief C++ program to print out CPU information
 * @copyright MIT License
 */

#include <cstdlib>
#include <ios>
#include <iostream>

#include "pdmath/cpu_info.h"

int main()
{
  pdmath::cpu_info info;
  std::cout <<
    "Vendor: " << info.vendor() << "\n" <<
    "Signature: " << std::hex << "0x" << info.signature() << "\n" <<
    "Features:\n" <<
    "  FPU           " << info.fpu() << "\n" <<
    "  CMOV          " << info.cmov() << "\n" <<
    "  MMX           " << info.mmx() << "\n" <<
    "  SSE           " << info.sse() << "\n" <<
    "  SSE2          " << info.sse2() << "\n" <<
    "  SSE3          " << info.sse3() << "\n" <<
    "  PCLMULQDQ     " << info.pclmulqdq() << "\n" <<
    "  SSSE3         " << info.ssse3() << "\n" <<
    "  FMA           " << info.fma() << "\n" <<
    "  SSE4.1        " << info.sse4_1() << "\n" <<
    "  SSE4.2        " << info.sse4_2() << "\n" <<
    "  POPCNT        " << info.popcnt() << "\n" <<
    "  AES           " << info.aes() << "\n" <<
    "  AVX           " << info.avx() << "\n" <<
    "  AVX2          " << info.avx2() << "\n" <<
    "  BMI1          " << info.bmi1() << "\n" <<
    "  BMI2          " << info.bmi2() << "\n" <<
    "  AVX-512F      " << info.avx512f()  << "\n" <<
    "  AVX-512DQ     " << info.avx512dq() << "\n" <<
    "  AVX-512IFMA   " << info.avx512ifma() << "\n" <<
    "  AVX-512PF     " << info.avx512pf() << "\n" <<
    "  AVX-512ER     " << info.avx512er() << "\n" <<
    "  AVX-512CD     " << info.avx512cd() << "\n" <<
    "  SHA           " << info.sha() << "\n" <<
    "  AVX-512BW     " << info.avx512bw() << "\n" <<
    "  AVX-512VL     " << info.avx512vl() << "\n" <<
    "  AVX-512VBMI   " << info.avx512vbmi() << "\n" <<
    "  AVX-512VBMI2  " << info.avx512vbmi2() << "\n" <<
    "  VAES          " << info.vaes() << "\n" <<
    "  VPCLMULQDQ    " << info.vpclmulqdq() << "\n" <<
    "  AVX-512VNNI   " << info.avx512vnni() << "\n"
    "  AMX-BF16      " << info.amxbf16() << "\n" <<
    "  AVX-512FP16   " << info.avx512fp16() << "\n" <<
    "  AMX-TILE      " << info.amxtile() << "\n" <<
    "  AMX-INT8      " << info.amxint8() << "\n" <<
    "  SHA-512       " << info.sha512() << "\n" <<
    "  AVX-VNNI      " << info.avxvnni() << "\n" <<
    "  AVX-IFMA      " << info.avxifma() << "\n" <<
    "  AVX10         " << info.avx10() << "\n" << std::flush;
  return EXIT_SUCCESS;
}
