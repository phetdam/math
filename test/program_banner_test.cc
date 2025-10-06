/**
 * @file program_banner_test.cc
 * @author Derek Huang
 * @brief program_banner.h unit tests
 * @copyright MIT License
 */

#include "pdmath/program_banner.h"

#include <cstddef>
#include <ostream>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "pdmath/type_traits.h"

namespace {

/**
 * Simple type for holding input and expected output string views.
 *
 * This provides a shorter type name than using a pair of `std::string_view`
 * and lets us elaborate as necessary. In this case, the input is a view to a
 * path-like string, and the output is the expected file name stem.
 */
class program_banner_case {
public:
  /**
   * Ctor.
   */
  constexpr program_banner_case(
    std::string_view input,
    std::string_view expected) noexcept
    : input_{input}, expected_{expected}
  {}

  /**
   * Return the input string view.
   */
  constexpr auto input() const noexcept { return input_; }

  /**
   * Return the expected string view file name stem result.
   */
  constexpr auto expected() const noexcept { return expected_; }

private:
  std::string_view input_;
  std::string_view expected_;
};

/**
 * Write the `program_banner_case` to an output stream.
 *
 * This is provided so Google Test can display `program_banner_case` instances
 * in a human-readable form instead of printing the bytes. Since CTest uses the
 * --gtest_list_tests output to append the `GetParam()` value to the test name,
 * not having this would result in very ugly, unreadable `TEST_P` CTest tests.
 */
auto& operator<<(std::ostream& out, const program_banner_case& value)
{
  return out << "{\"" << value.input() << "\", \"" << value.expected() << "\"}";
}

/**
 * Test fixture for runtime program banner testing.
 *
 * @note We can use string views here since they are initialized with literals.
 */
class ProgramBannerTest : public ::testing ::TestWithParam<program_banner_case> {};

/**
 * Test that the `program_banner` computes the file stem correctly.
 */
TEST_P(ProgramBannerTest, StemTest)
{
  // reference to parameter
  const auto& param = GetParam();
  // check equality of stem
  EXPECT_EQ(param.expected(), pdmath::program_banner{param.input()}.stem());
}

// input values + registration macro. this allows us to avoid repeating the
// type name each time we are interested in adding a new input case
constexpr program_banner_case stem_test_cases[] = {
  {"/path/to/myfile", "myfile"},           // POSIX abspath (no extension)
  {"/path/to/.dotfile", ".dotfile"},       // POSIX abspath (dotfile)
  {"/path/to/file.txt", "file"},           // POSIX abspath (with extension)
  {"rel/to/file.txt", "file"},             // POSIX relpath (with extension)
  {"rel/to/.dotfile", ".dotfile"},         // POSIX relpath (dotfile)
  {"rel/to/file", "file"},                 // POSIX relpath (no extension)
  {"file.txt", "file"},                    // file name -> stem
  {"C:\\path\\to\\myfile", "myfile"},      // Windows abspath (no extension)
  {"C:\\path\\to\\.dotfile", ".dotfile"},  // Windows abspath (dotfile)
  {"D:\\path\\to\\file.txt", "file"},      // Windows abspath (with extension)
  {"path\\to\\file.txt", "file"},          // Windows relpath (with extension)
  {"path\\to\\.dotfile", ".dotfile"},      // Windows relpath (dotfile)
  {"path\\to\\file", "file"},              // Windows relpath (no extension)
  {__FILE__, "program_banner_test"}        // __FILE__
};

INSTANTIATE_TEST_SUITE_P(
  Core,
  ProgramBannerTest,
  ::testing::ValuesIn(stem_test_cases)
);

/**
 * Compile-time test input for `program_banner`.
 *
 * This ensures that the `program_banner` is `constexpr` as advertised.
 *
 * @tparam I Index into `stem_test_cases`
 */
template <std::size_t I>
struct program_banner_constexpr_case {
  // test index, banner object, and expected file stem
  static constexpr auto index = I;
  static constexpr pdmath::program_banner value{stem_test_cases[I].input()};
  static constexpr auto expected = stem_test_cases[I].expected();
};

/**
 * Test fixture template for compile-time `program_banner` test cases.
 *
 * @tparam T `program_banner_constexpr_case<I>`
 */
template <typename T>
class ProgramBannerConstexprTest : public ::testing::Test {};

/**
 * Traits impl to deduce the `::testing::Types` for the compile-time tests.
 *
 * @tparam T `std::index_sequence`
 */
template <typename T>
struct program_banner_constexpr_traits_impl {};

/**
 * Partial specialization to deduce the index pack.
 *
 * @tparam Is Indices 0 through the last indexc in `stem_test_cases`
 */
template <std::size_t... Is>
struct program_banner_constexpr_traits_impl<std::index_sequence<Is...>> {
  using type = ::testing::Types<program_banner_constexpr_case<Is>...>;
};

/**
 * Traits to deduce the `::testing::Types` for the compile-time tests.
 *
 * @tparam N Number of test cases
 */
template <std::size_t N>
struct program_banner_constexpr_traits :
  program_banner_constexpr_traits_impl<std::make_index_sequence<N>> {};

// register ProgramBannerConstexprTest types
TYPED_TEST_SUITE(
  ProgramBannerConstexprTest,
  typename program_banner_constexpr_traits<std::size(stem_test_cases)>::type
);

/**
 * Check that the `program_banner` computes the stem at compile-time correctly.
 *
 * @note This is mostly a smoke check of the compile-time result.
 */
TYPED_TEST(ProgramBannerConstexprTest, StemTest)
{
  EXPECT_EQ(TypeParam::expected, TypeParam::value.stem());
}

}  // namespace
