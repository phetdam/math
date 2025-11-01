/**
 * @file simd_test.cc
 * @author Derek Huang
 * @brief simd.h unit tests
 * @copyright MIT License
 */

#include "pdmath/simd.h"

#include <string>
#include <type_traits>

#include <gtest/gtest.h>

#include "pdmath/features.h"
#include "pdmath/testing/utils.h"
#include "pdmath/warnings.h"

namespace {

/**
 * Test fixture template for testing `is_simd_type`.
 *
 * @tparam T `type_value_pair` specialization
 */
template <typename T>
class IsSimdTypeTest
  : public pdmath::testing::traits_test<pdmath::is_simd_type, T> {};

/**
 * Name generator for `IsSimdTypeTest`.
 */
class SimdNameGenerator {
  /**
   * Name generation function implementation CPO.
   *
   * We use a CPO since function templates cannot be partially specialized.
   */
  template <typename U>
  struct name_generator {};

  /**
   * Partial specialization for `type_value_pair`.
   *
   * @tparam U type
   * @tparam v value
   */
  template <typename U, auto v>
  struct name_generator<pdmath::testing::type_value_pair<U, v>> {
    /**
     * Returns either a SIMD type name or the ordinal case number.
     *
     * We follow the Google Test naming convention so no leading underscores.
     */
    std::string operator()(int i) const
    {
// GCC warns about attributes being ignored on template arguments. this is due
// to how GCC implements the intrinsic SIMD types
PDMATH_GNU_WARNINGS_PUSH()
PDMATH_GNU_WARNINGS_DISABLE("-Wignored-attributes")
#if PDMATH_HAS_SSE
      if constexpr (std::is_same_v<U, __m128>)
        return "m128";
      else if constexpr (std::is_same_v<U, __m128d>)
        return "m128d";
      else if constexpr (std::is_same_v<U, __m128i>)
        return "m128i";
      else
#endif  // PDMATH_HAS_SSE
#if PDMATH_HAS_AVX
      if constexpr (std::is_same_v<U, __m256>)
        return "m256";
      else if constexpr (std::is_same_v<U, __m256d>)
        return "m256d";
      else if constexpr (std::is_same_v<U, __m256i>)
        return "m256i";
      else
#endif  // PDMATH_HAS_AVX
#if PDMATH_HAS_AVX512F
      if constexpr (std::is_same_v<U, __m512>)
        return "m512";
      else if constexpr (std::is_same_v<U, __m512d>)
        return "m512d";
      else if constexpr (std::is_same_v<U, __m512i>)
        return "m512i";
      else
#endif  // PDMATH_AVX512F
PDMATH_GNU_WARNINGS_POP()
        return std::to_string(i);
    }
  };

public:
  /**
   * Return the `TYPED_TEST` test case name to use given the type and ordinal.
   *
   * @tparam T type
   *
   * @param i Ordinal value for the test case type
   */
  template <typename T>
  static auto GetName(int i)
  {
    return name_generator<T>{}(i);
  }
};

// GCC warns about attributes being ignored on template arguments
PDMATH_GNU_WARNINGS_PUSH()
PDMATH_GNU_WARNINGS_DISABLE("-Wignored-attributes")
using IsSimdTypeTestTypes = ::testing::Types<
#if PDMATH_HAS_SSE
  pdmath::testing::type_value_pair<__m128, true>,
  pdmath::testing::type_value_pair<__m128d, true>,
  pdmath::testing::type_value_pair<__m128i, true>,
#endif  // PDMATH_HAS_SSE
#if PDMATH_HAS_AVX
  pdmath::testing::type_value_pair<__m256, true>,
  pdmath::testing::type_value_pair<__m256d, true>,
  pdmath::testing::type_value_pair<__m256i, true>,
#endif  // PDMATH_HAS_AVX
#if PDMATH_HAS_AVX512F
  pdmath::testing::type_value_pair<__m512, true>,
  pdmath::testing::type_value_pair<__m512d, true>,
  pdmath::testing::type_value_pair<__m512i, true>,
#endif  // PDMATH_HAS_AVX512F
  pdmath::testing::type_value_pair<int, false>,
  pdmath::testing::type_value_pair<const void*, false>
>;
PDMATH_GNU_WARNINGS_POP()
TYPED_TEST_SUITE(IsSimdTypeTest, IsSimdTypeTestTypes, SimdNameGenerator);

// define is_simd_type test
PDMATH_TRAITS_TEST(IsSimdTypeTest);

}  // namespace
