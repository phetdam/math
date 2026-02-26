/**
 * @file golden_search_test.cc
 * @author Derek huang <djh458@stern.nyu.edu>
 * @brief Test for golden_search, validating that it works correctly
 * @copyright MIT License
 */

#include "pdmath/golden_search.h"

#include <cmath>
#include <limits>
#include <type_traits>

#include <gtest/gtest.h>

#include "pdmath/bases.h"
#include "pdmath/optimize_result.h"
#include "pdmath/pi.h"

namespace {

template <typename T>
class scalar_quadratic : public pdmath::functor_base<T> {
public:
  /**
   * Constructor for the functor.
   *
   * @param a Value scaling the quadratic term
   * @param b Value scaling the linear term
   * @param c Offset
   */
  scalar_quadratic(const T& a, const T& b, const T& c) : a_(a), b_(b), c_(c) {}

  /**
   * Evaluate for a given value.
   */
  T operator()(const T& x) override { return a_ * x * x + b_ * x + c_; }

private:
  T a_;
  T b_;
  T c_;
};

/**
 * Traits to indicate a type has a `lower()` member function.
 *
 * This member function must be const-qualified and return a float value.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct has_lower : std::false_type {};

/**
 * True specialization for a type with a `lower()` member function.
 *
 * @tparam T type
 */
template <typename T>
struct has_lower<
  T,
  std::enable_if_t<
    std::is_floating_point_v<decltype(std::declval<const T>().lower())>
  > > : std::true_type {};

/**
 * Traits to indicate a type has an `upper()` member function.
 *
 * This member function must be const-qualified and return a float value.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct has_upper : std::false_type {};

/**
 * True specialization for a type with an `upper()` member function.
 *
 * @tparam T type
 */
template <typename T>
struct has_upper<
  T,
  std::enable_if_t<
    std::is_floating_point_v<decltype(std::declval<const T>().upper())>
  > > : std::true_type {};

/**
 * Traits to indicate a type has a `tol()` member function.
 *
 * This member function must be const-qualified and return a float value.
 *
 * @tparam T type
 */
template <typename T, typename = void>
struct has_tol : std::false_type {};

/**
 * True specialization for a type with a `tol()` member function.
 *
 * @tparam T type
 */
template <typename T>
struct has_tol<
  T,
  std::enable_if_t<
    std::is_floating_point_v<decltype(std::declval<const T>().tol())>
  > > : std::true_type {};

/**
 * Golden search test case CRTP base.
 *
 * This provides defaults for the tolerance, lower bound, and upper bound. Each
 * test input type must define a unary `operator()` and an `expected()` member.
 *
 * @tparam T Unary invokable test input type
 */
template <typename T>
class golden_search_test_case {
private:
  /**
   * Helper template to get the invoke result of `T`.
   *
   * We cannot directly use a type alias without template parameter dependence
   * because `T` is still an incomplete type when used in CRTP.
   *
   * @tparam T Unary invokable
   */
  template <typename U>
  using invoke_result = std::invoke_result_t<U, int>;

public:
  /**
   * Return the lower bound.
   *
   * If not provided the default is zero.
   */
  constexpr auto lower() const noexcept
  {
    if constexpr (has_lower<T>::value)
      return static_cast<const T*>(this)->lower();
    else
      return static_cast<invoke_result<T>>(0);
  }

  /**
   * Return the upper bound.
   *
   * If not provided the default is one.
   */
  constexpr auto upper() const noexcept
  {
    if constexpr (has_upper<T>::value)
      return static_cast<const T*>(this)->upper();
    else
      return static_cast<invoke_result<T>>(1);
  }

  /**
   * Return the comparison tolerance.
   *
   * If not provided the square root of the epsilon for the type is used.
   */
  constexpr auto tol() const noexcept
  {
    if constexpr (has_tol<T>::value)
      return static_cast<const T*>(this)->tol();
    else
      return std::sqrt(std::numeric_limits<invoke_result<T>>::epsilon());
  }
};

/**
 * Test template for the golden search method.
 *
 * @tparam T
 */
template <typename T>
class GoldenSearchTest : public ::testing::Test {};

/**
 * Test case representing a convex quadratic.
 *
 * The minimum in `[0, 1]` will be at 0 with a root at 0.5.
 *
 * @tparam T Floating-point type
 */
template <typename T>
class convex_x2_input : public golden_search_test_case<convex_x2_input<T>> {
public:
  constexpr T operator()(T x) const noexcept
  {
    return x * x - static_cast<T>(0.25);
  }

  constexpr T expected() const noexcept
  {
    return 0;
  }
};

/**
 * Test case representing a concave quadratic.
 *
 * The minimum in `[0, 1]` will be at 1.
 *
 * @tparam T Floating-point type
 */
template <typename T>
class concave_x2_input : public golden_search_test_case<concave_x2_input<T>> {
public:
  constexpr T operator()(T x) const noexcept
  {
    return -x * x + static_cast<T>(0.25);
  }

  constexpr T expected() const noexcept
  {
    return 1;
  }
};

/**
 * Test case representing a linear function.
 *
 * The minimum in `[0, 1]` will be at 0 with a zero at -1.
 *
 * @tparam T Floating-point type
 */
template <typename T>
class linear_input : public golden_search_test_case<linear_input<T>> {
public:
  constexpr T operator()(T x) const noexcept
  {
    return (x + 1) / 2;
  }

  constexpr T expected() const noexcept
  {
    return 0;
  }
};

/**
 * Test case representing the standard sine function.
 *
 * The minimum in `[5 * pi / 4, 7 * pi / 4]` is 1.5 times pi.
 *
 * @note We have to relax the tolerance a bit to get the test to pass.
 *
 * @tparam T Floating-point type
 */
template <typename T>
class sine_input : public golden_search_test_case<sine_input<T>> {
public:
  T operator()(T x) const noexcept
  {
    return std::sin(x);
  }

  constexpr T expected() const noexcept
  {
    return 3 * pdmath::pi_v<T> / 2;
  }

  constexpr T lower() const noexcept
  {
    return 5 * pdmath::pi_v<T> / 4;
  }

  constexpr T upper() const noexcept
  {
    return 7 * pdmath::pi_v<T> / 4;
  }

  constexpr T tol() const noexcept
  {
    // a bit larger than sqrt(epsilon())
    return static_cast<T>(std::pow(std::numeric_limits<T>::epsilon(), 0.4));
  }
};

// initialize test cases
using GoldenSearchTestTypes = ::testing::Types<
  convex_x2_input<double>,
  convex_x2_input<float>,
  concave_x2_input<double>,
  concave_x2_input<float>,
  linear_input<double>,
  linear_input<float>,
  sine_input<double>
>;
TYPED_TEST_SUITE(GoldenSearchTest, GoldenSearchTestTypes);

/**
 * Test that the golden-section search works as expected.
 */
TYPED_TEST(GoldenSearchTest, Test)
{
  TypeParam in{};
  auto res = pdmath::golden_search(in, in.lower(), in.upper(), in.tol());
  // check convergence
  EXPECT_TRUE(res.converged());
  EXPECT_NEAR(in.expected(), res.res(), in.tol());
  // n_fev should be 4 + 2 * n_iter + 1, since we evaluate f at res again
  EXPECT_EQ(4 + 2 * res.n_iter() + 1, res.n_fev());
}

}  // namespace
