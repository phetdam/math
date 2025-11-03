/**
 * @file cpu_info.h
 * @author Derek Huang
 * @brief C++ header for CPU feature detection
 * @copyright MIT License
 */

#ifndef PDMATH_CPUINFO_H_
#define PDMATH_CPUINFO_H_

// for __cpuid, __cpuidex
#if defined(_WIN32)
#define PDMATH_HAS_CPUID 1
#include <intrin.h>
// use cpuid.h for GNU-like compilers
#elif defined(__GNUC__)
#define PDMATH_HAS_CPUID 1
#include <cpuid.h>
#else
#define PDMATH_HAS_CPUID 0
#endif  // !defined(_WIN32) && !defined(__GNUC__)

#include <climits>
#include <cstdint>
#include <type_traits>

#include "pdmath/warnings.h"

namespace pdmath {

/**
 * Integral type used for the corresponding `cpuid` intrinsic.
 *
 * Windows uses plain `int` while GCC uses `unsigned`.
 */
#if defined(_WIN32)
    using cpuid_int = int;
#else
    using cpuid_int = unsigned;
#endif  // !defined(_WIN32)

/**
 * Call `cpuid` and fill register info.
 *
 * This provides a convenient `cpuid` wrapper that always has the value of the
 * ecx subleaf appropriately set. This prevents confusing results between the
 * Windows `__cpuid` and the GCC `__get_cpuid` as the Windows `__cpuid` clears
 * ecx while GCC's `__get_cpuid` doesn't. Therefore, for leaf values that
 * require ecx be set to a specific value, e.g. 7, `__get_cpuid` incorrectly
 * appears to not work while `__cpuid` works (since it clears ecx).
 *
 * @param regs Array representing eax, ebx, ecx, edx in order
 * @param leaf `cpuid` calling parameter (leaf) for eax
 * @param sub `cpuid` calling parameter (subleaf) for ecx
 * @returns `true` if `cpuid` is supported, `false` otherwise
 */
inline bool cpuid(cpuid_int (&regs)[4], cpuid_int leaf = 0, cpuid_int sub = 0)
{
#if defined(_WIN32)
  __cpuidex(regs, leaf, sub);
  return true;
#elif defined(__GNUC__)
  return !!__get_cpuid_count(leaf, sub, &regs[0], &regs[1], &regs[2], &regs[3]);
#else
  return false;
#endif  // !defined(_WIN32) && !defined(__GNUC__)
}

namespace detail {

/**
 * Check that a given bit has been set.
 *
 * @tparam I Bit position to test
 * @tparam T Integral type
 */
template <unsigned I, typename T>
constexpr bool test(
  T v,
  std::enable_if_t<std::is_integral_v<T>, int> = 0) noexcept
{
  // safety check so we don't go out-of-bounds
  static_assert(I < CHAR_BIT * sizeof(T), "I >= CHAR_BIT * sizeof(T)");
  return !!(v & (0x1 << I));
}

}  // namespace detail

/**
 * CPU info structure.
 *
 * This holds the feature flags returned by `cpuid` for both basic and extended
 * features. Currently, the following is provided if possible:
 *
 *  1.  %eax = 0 info (vendor, highest cpuid %eax parameter value)
 *  2.  %eax = 1 %ecx, %edx info
 *  2.  %eax = 7 %ebx, %ecx, %edx info (more features, e.g. AVX support, etc)
 *
 * @note CPU feature detection only supported for x86-64 systems. On ARM
 *  systems all the features flags will be empty and no vendor name is
 *  provided. This may change in the future when I get access to ARM chips.
 */
class cpu_info {
public:
  /**
   * Ctor.
   *
   * This calls the corresponding `cpuid` instruction if available.
   */
  cpu_info() noexcept
  {
    // eax, ebx, ecx, edx registers
    cpuid_int regs[4u];
    // call cpuid with eax = 0 for basic info. if unsupported, return
    if (!cpuid(regs))
      return;
    // extract vendor string from ebx, edx, ecx
    // note: yes, the order is ebx, edx, ecx
    *reinterpret_cast<cpuid_int*>(vendor_) = regs[1];
    *reinterpret_cast<cpuid_int*>(vendor_ + 4u) = regs[3];
    *reinterpret_cast<cpuid_int*>(vendor_ + 8u) = regs[2];
    // update max leaf value from eax
    max_leaf_ = static_cast<unsigned>(regs[0]);
    // if max leaf >= 1, get info
    if (max_leaf_ >= 1u) {
      cpuid(regs, 1);
// MSVC complains about signed/unsigned mismatch
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4365)
      eax_1_ = regs[0];
      ecx_1_ = regs[2];
      edx_1_ = regs[3];
PDMATH_MSVC_WARNINGS_POP()
    }
    // if max leaf >= 7, get info
    if (max_leaf_ >= 7u) {
      // eax = 7, ecx = 0
      cpuid(regs, 7);
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4365)
      ebx_7_ = regs[1];
      ecx_7_ = regs[2];
      edx_7_ = regs[3];  // TODO: unused for now
PDMATH_MSVC_WARNINGS_POP()
      // save max eax = 7 subleaf value
      auto max_subleaf_7 = static_cast<unsigned>(regs[0]);
      // eax = 7, ecx = 1
      if (max_subleaf_7 >= 1u) {
        cpuid(regs, 7, 1);
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4365)
        eax_7_1_ = regs[0];
        edx_7_1_ = regs[3];
PDMATH_MSVC_WARNINGS_POP()
      }
    }
    // if max leaf >= 0x24 (36), get info
    // note: max supported subleaf for eax = 0x24 is in regs[0]
    if (max_leaf_ >= 0x24) {
      cpuid(regs, 0x24);
      // only lower 8 bits matter in ebx
      avx10_ver_ = static_cast<unsigned char>(regs[1] & 0xFF);
    }
  }

  /**
   * Get the null-terminated CPU vendor string.
   */
  auto vendor() const noexcept { return vendor_; }

  /**
   * Get the maximum basic leaf parameter value.
   */
  auto max_leaf() const noexcept { return max_leaf_; }

  /**
   * Return the CPU signature.
   *
   * This is the value of eax after `cpuid` is called with leaf 1.
   */
  auto signature() const noexcept { return eax_1_; }

  /**
   * `fpu` feature flag indicating if an x87 FPU is available.
   */
  bool fpu() const noexcept { return detail::test<0>(edx_1_); }

  /**
   * `cmov` feature flag indicating if conditional move is available.
   */
  bool cmov() const noexcept { return detail::test<15>(edx_1_); }

  /**
   * `mmx` feature flag indicating if MMX instructions are available.
   */
  bool mmx() const noexcept { return detail::test<23>(edx_1_); }

  /**
   * `sse` feature flag indicating if SSE instructions are available.
   */
  bool sse() const noexcept { return detail::test<25>(edx_1_); }

  /**
   * `sse2` feature flag indicating if SSE2 instructions are available.
   */
  bool sse2() const noexcept { return detail::test<26>(edx_1_); }

  /**
   * `sse3` feature flag indicating if SSE3 instructions are available.
   */
  bool sse3() const noexcept { return detail::test<0>(ecx_1_); }

  /**
   * `pclmulqdq` feature flag to check if `PCLMULQDQ` is available.
   */
  bool pclmulqdq() const noexcept { return detail::test<1>(ecx_1_); }

  /**
   * `ssse3` feature flag indicating if SSSE3 instructions are available.
   *
   * @note Not to be confused with SSE3 instructions.
   */
  bool ssse3() const noexcept { return detail::test<9>(ecx_1_); }

  /**
   * `fma` feature flag indicating if FMA3 instructions are available.
   */
  bool fma() const noexcept { return detail::test<12>(ecx_1_); }

  /**
   * `sse4.1` feature flag indicating if SSE4.1 instructions are available.
   */
  bool sse4_1() const noexcept { return detail::test<19>(ecx_1_); }

  /**
   * `sse4.2` feature flag indicating if SSE4.2 instructions are available.
   */
  bool sse4_2() const noexcept { return detail::test<20>(ecx_1_); }

  /**
   * `popcnt` feature flag indicating if `POPCNT` is available.
   */
  bool popcnt() const noexcept { return detail::test<23>(ecx_1_); }

  /**
   * `aes-ni` feature flag indiacting if AES instructions are available.
   */
  bool aes() const noexcept { return detail::test<25>(ecx_1_); }

  /**
   * `avx` feature flag indicating if AVX instructions are available.
   */
  bool avx() const noexcept { return detail::test<28>(ecx_1_); }

  /**
   * `f16c` feature flag indicating if FP16 conversions are available.
   */
  bool f16c() const noexcept { return detail::test<29>(ecx_1_); }

  /**
   * `rdrnd` feature flag indicating if `RDRAND` is available.
   */
  bool rdrnd() const noexcept { return detail::test<30>(ecx_1_); }

  /**
   * `bmi1` feature flag indicating if BMI1 instructions are available.
   */
  bool bmi1() const noexcept { return detail::test<3>(ebx_7_); }

  /**
   * `avx2` feature flag indicating if AVX2 instructions are available.
   */
  bool avx2() const noexcept { return detail::test<5>(ebx_7_); }

  /**
   * `bmi2` feature flag indicating if BMI2 instructions are available.
   */
  bool bmi2() const noexcept { return detail::test<8>(ebx_7_); }

  /**
   * `avx512-f` flag indicating if AVX-512F instructions are available.
   */
  bool avx512f() const noexcept { return detail::test<16>(ebx_7_); }

  /**
   * `avx512-dq` flag indicating if AVX-512DQ instructions are available.
   */
  bool avx512dq() const noexcept { return detail::test<17>(ebx_7_); }

  /**
   * `avx512-ifma` flag indicating if AVX-512IFMA instructions are available.
   */
  bool avx512ifma() const noexcept { return detail::test<21>(ebx_7_); }

  /**
   * `avx512-pf` flag indicating if AVX-512PF instructions are available.
   */
  bool avx512pf() const noexcept { return detail::test<26>(ebx_7_); }

  /**
   * `avx512-er` flag indicating if AVX-512ER instructions are available.
   */
  bool avx512er() const noexcept { return detail::test<27>(ebx_7_); }

  /**
   * `avx512-cd` flag indicating if AVX-512CD instructions are available.
   */
  bool avx512cd() const noexcept { return detail::test<28>(ebx_7_); }

  /**
   * `sha` flag indicating if SHA-1 and SHA-256 instructions are available.
   */
  bool sha() const noexcept { return detail::test<29>(ebx_7_); }

  /**
   * `avx512-bw` flag indicating if AVX-512BW instructions are available.
   */
  bool avx512bw() const noexcept { return detail::test<30>(ebx_7_); }

  /**
   * `avx512-vl` flag indicating if AVX-512VL instructions are available.
   */
  bool avx512vl() const noexcept { return detail::test<31>(ebx_7_); }

  /**
   * `avx512-vbmi` flag indicating if AVX-512VBMI instructions are available.
   */
  bool avx512vbmi() const noexcept { return detail::test<1>(ecx_7_); }

  /**
   * `avx512-vbmi2` flag indicating if AVX-512VBMI2 instructions are available.
   */
  bool avx512vbmi2() const noexcept { return detail::test<6>(ecx_7_); }

  /**
   * `vaes` feature flag indicating if vector AES instructions are available.
   */
  bool vaes() const noexcept { return detail::test<9>(ecx_7_); }

  /**
   * `vpclmulqdq` flag to check if `VPCLMULQDQ` is available.
   */
  bool vpclmulqdq() const noexcept { return detail::test<10>(ecx_7_); }

  /**
   * `avx512-vnni` flag indicating if AVX-512VNNI instructions are available.
   */
  bool avx512vnni() const noexcept { return detail::test<11>(ecx_7_); }

  // TODO: add more AVX-512 testers here, e.g. for BITALG and VPOPCOUNTDQ

  /**
   * `amx-bf16` flag indicating if AMX bfloat16 support is available.
   */
  bool amxbf16() const noexcept { return detail::test<22>(edx_7_); }

  /**
   * `avx512-fp16` flag indicating if AVX-512FP16 instructions are present.
   */
  bool avx512fp16() const noexcept { return detail::test<23>(edx_7_); }

  /**
   * `amx-tile` flag indicating if AMX tile load/store is available.
   */
  bool amxtile() const noexcept { return detail::test<24>(edx_7_); }

  /**
   * `amx-int8` flag indicatinf if AMX int8 support is available.
   */
  bool amxint8() const noexcept { return detail::test<25>(edx_7_); }

  /**
   * `sha512` feature flag indicating if SHA-512 instructions are available.
   */
  bool sha512() const noexcept { return detail::test<0>(eax_7_1_); }

  /**
   * `avx-vnni` flag indicating if AVX-VNNI instructions are available.
   */
  bool avxvnni() const noexcept { return detail::test<4>(eax_7_1_); }

  /**
   * `avx-ifma` flag indicating if AVX-IFMA instructions are available.
   */
  bool avxifma() const noexcept { return detail::test<23>(eax_7_1_); }

  /**
   * `avx10` feature flag indicating if AVX10 instructions are available.
   */
  bool avx10() const noexcept { return detail::test<19>(edx_7_1_); }

  /**
   * Indicate if AVX10.1 instructions are available.
   */
  bool avx10_1() const noexcept { return detail::test<0>(avx10_ver_); }

  /**
   * Indicate if AVX10.2 instructions are available.
   */
  bool avx10_2() const noexcept { return detail::test<1>(avx10_ver_); }

private:
  char vendor_[13]{};          // vendor string (null-terminated)
  unsigned max_leaf_{};        // max basic eax parameter
  std::uint32_t eax_1_{};      // eax = 1 cpuid eax info
  std::uint32_t ecx_1_{};      // eax = 1 cpuid ecx info
  std::uint32_t edx_1_{};      // eax = 1 cpuid edx info
  std::uint32_t ebx_7_{};      // eax = 7, ecx = 0 cpuid ebx info
  std::uint32_t ecx_7_{};      // eax = 7, ecx = 0 cpuid ecx info
  std::uint32_t edx_7_{};      // eax = 7, ecx = 0 cpuid edx info
  std::uint32_t eax_7_1_{};    // eax = 7, ecx = 1 cpuid eax info
  std::uint32_t edx_7_1_{};    // eax = 7, ecx = 1 cpuid edx info
  unsigned char avx10_ver_{};  // eax = 0x24, ecx = 0 cpuid AVX10 version
};

}  // namespace pdmath

#endif  // PDMATH_CPUINFO_H_
