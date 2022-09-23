/**
 * @file norms_tests.cc
 * @author Derek Huang
 * @brief Tests for norm functors
 * @copyright MIT License
 */

#include "pdmath/norms.h"

#include <cmath>
#include <numeric>
#include <type_traits>

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "pdmath/helpers.h"
#include "pdmath/types.h"

namespace {

/**
 * Magnitude of largest value in `NORMS_TEST_VECTOR_VALUES`.
 */
#define NORMS_TEST_VALUES_ABS_MAX 8.1

/**
 * Values used in the `NormsTest` test fixture for populating `std::vector`.
 *
 * Used later in `MaxNormTest` test to test the `boost_vector<double>` and
 * `eigen_vector<double>` specializations of `max_norm`.
 */
#define NORMS_TEST_VECTOR_VALUES -5.7, 6, -NORMS_TEST_VALUES_ABS_MAX, 1.3

/**
 * Template class rolling together a *Container* and an `unsigned int`.
 *
 * We use this class with `TYPED_TEST` to provide access to an `unsigned int`
 * value for `p` used in `PNormTest` since `TYPED_TEST` and `TEST_P` can't be
 * combined, as the former uses a template class unsuitable for the latter.
 *
 * @tparam V_t *Container*
 * @tparam p `unsigned int`
 */
template <typename V_t, unsigned int p>
class norm_param_pair {
public:
  PDMATH_USING_CONTAINER_TYPES(V_t);
  norm_param_pair() = delete;
  static constexpr unsigned int p_ = p;
};

/**
 * Templated test fixture for norm tests.
 *
 * @tparam Tp `norm_param_pair<V_t, unsigned int>` where `V_t` is a *Container*
 *     with `typename V_t::value_type` equal to `double`
 */
template <typename Tp>
class NormsTest : public ::testing::Test {
  using element_type = typename Tp::element_type;
protected:
  // to keep the tests simple, we require value_type to be double. this way we
  // do not need to have different float/double comparison tolerances.
  static_assert(std::is_same_v<double, element_type>);
  static const typename Tp::container_type values_;
  static constexpr element_type values_abs_max_ = NORMS_TEST_VALUES_ABS_MAX;
};

// use of {{}} constructor ensures use of initializer list constructor
template <typename Tp>
const typename Tp::container_type NormsTest<Tp>::values_{
  {NORMS_TEST_VECTOR_VALUES}
};

using NormsTestTypes = ::testing::Types<
// hack: MSVC incorrectly emits compile failure for line 119 when p_ is 0
#ifdef _MSC_VER
  norm_param_pair<pdmath::vector_d, 1>,
#else
  norm_param_pair<pdmath::vector_d, 0>,
#endif  // _MSC_VER
  norm_param_pair<Eigen::VectorXd, 1>,
  norm_param_pair<pdmath::array_d<4>, 2>,
  norm_param_pair<Eigen::Vector4d, 3>
>;
TYPED_TEST_SUITE(NormsTest, NormsTestTypes);

/**
 * Test that the max norm works as intended.
 */
TYPED_TEST(NormsTest, MaxNormTest)
{
  pdmath::max_norm<typename TypeParam::container_type> norm;
  EXPECT_DOUBLE_EQ(NORMS_TEST_VALUES_ABS_MAX, norm(TestFixture::values_));
}

/**
 * Test that the p-norm works as intended for different `p` values.
 *
 * When `p` is zero, this is the max norm case.
 */
TYPED_TEST(NormsTest, PNormTest)
{
  using element_type = typename TypeParam::element_type;
  pdmath::p_norm<typename TypeParam::container_type> norm(TypeParam::p_);
  // compute expected norm
  element_type e_norm = std::accumulate(
    TestFixture::values_.cbegin(),
    TestFixture::values_.cend(),
    0.,
    [](const element_type& tgt, const element_type& x)
    {
      return tgt + std::abs(std::pow(x, TypeParam::p_));
    }
  );
  // for max norm, we don't take any fractional power
  // compilation error in MSVC when p_ is 0 -- report? works in GCC just fine
  e_norm = (TypeParam::p_) ? std::pow(e_norm, 1. / TypeParam::p_) : e_norm;
  // compare against actual computed by norm
  ASSERT_DOUBLE_EQ(e_norm, norm(TestFixture::values_));
}

}  // namespace
