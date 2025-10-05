/**
 * @file helpers.h
 * @author Derek Huang
 * @brief C++ header for common helpers
 * @copyright MIT License
 */

#ifndef PDMATH_HELPERS_H_
#define PDMATH_HELPERS_H_

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <Eigen/Core>

#include "pdmath/bases.h"
#include "pdmath/type_traits.h"
#include "pdmath/types.h"
#include "pdmath/warnings.h"

namespace pdmath {

/**
 * Print a short banner header with the file name of the example program run.
 *
 * Useful for identifying the program in `examples` being run by the user.
 *
 * @note This function is really not all that helpful. In the future we should
 *  should have something that creates a similar banner with just the file name
 *  and no attempt to have other intermediate paths.
 *
 * @param path Absolute path of program with `main`, e.g. `__FILE__`
 * @param print `true` to print header with an extra `\n`
 * @returns `std::string` giving the header to print
 */
inline auto print_example_header(std::string_view path, bool print = true)
{
  constexpr std::string_view split_str{"math/examples"};
  // the +1 at the end is to not include the directory slash
  auto rel_path = path.substr(path.find(split_str) + split_str.size() + 1);
  // add 2 to frame since we pad with a space on each side of rpath
  std::string frame(rel_path.size() + 2u, '-');
  // create the final header
  std::stringstream ss;
  ss << "+" << frame << "+\n| " << rel_path << " |\n+" << frame << "+";
  auto header = ss.str();
  if (print)
    std::cout << header << std::endl;
  return header;
}

/**
 * Create a new Boost vector from a *Container*.
 *
 * @tparam V_t *Container* to copy elements from
 *
 * @param from `const V_t&` *Container* whose elements we will copy
 */
template <typename V_t>
inline boost_vector<typename V_t::value_type> boost_vector_from(const V_t& from)
{
// MSVC complains about signed to unsigned conversion if using Eigen vectors
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4365)
#endif  // _MSC_VER
  boost_vector<typename V_t::value_type> to(from.size());
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif  // _MSC_VER
  // note: when compiling with MSVC, using indexing when copying using a for
  // loop causes C5045 to be emitted with /Wall, but not with iterators.
  std::copy(from.cbegin(), from.cend(), to.begin());
  return to;
}

/**
 * Create a new Boost vector with type `T` from a list of arguments.
 *
 * Uses a `static_cast<T>` to convert the arguments.
 *
 * Since the template `T` argument cannot be deduced, the recommended usage is:
 *
 * `auto bvec = boost_vector_from<T>(arg1, ... argN)`
 *
 * @tparam T `boost_vector` element type
 * @tparam Ts parameter pack
 *
 * @param from `Ts...` list of arguments that can be cast to `T`
 */
template <typename T, typename... Ts>
inline boost_vector<T> boost_vector_from(Ts... from)
{
  return boost_vector_from(std::vector<T>({static_cast<T>(from)...}));
}

/**
 * Create a new Eigen dynamic vector with type `T` from a list of arguments.
 *
 * Uses a `static_cast<T>` to convert the arguments.
 *
 * Since the template `T` argument cannot be deduced, the recommended usage is:
 *
 * `auto evec = eigen_vector_from<T>(arg1, ... argN);`
 *
 * @tparam T `Eigen::VectorX` element type
 * @tparam Ts parameter pack
 *
 * @param from `const Ts&...` list of arguments that can be cast to `T`
 */
template <typename T, typename... Ts>
inline auto eigen_vector_from(const Ts&... from)
{
  return Eigen::VectorX<T>{{static_cast<T>(from)...}};
}

/**
 * Create a new Eigen dynamic vector from any `Eigen::MatrixBase`.
 *
 * 2D matrix elements will be read in expected row-major order.
 *
 * @tparam Derived The derived matrix/matrix expression type
 *
 * @param from `const Eigen::MatrixBase<T>&` to create new vector from
 */
template <typename Derived>
inline Eigen::VectorX<typename Derived::Scalar> eigen_vector_from(
  const Eigen::MatrixBase<Derived>& from)
{
  Eigen::VectorX<typename Derived::Scalar> to(from.size());
  // reshape returns a view, not actually moving elements around
  auto flat_view = from.transpose().reshaped();
  std::copy(flat_view.cbegin(), flat_view.cend(), to.begin());
  return to;
}

/**
 * Create a new Eigen dynamic vector from a `std::vector<T>`.
 *
 * @tparam T element type
 *
 * @param from `const std::vector<T>&` vector whose elements we will copy
 */
template <typename T>
inline Eigen::VectorX<T> eigen_vector_from(const std::vector<T>& from)
{
  Eigen::VectorX<T> to(from.size());
  std::copy(from.cbegin(), from.cend(), to.begin());
  return to;
}

/**
 * Create a new Eigen dynamic vector from a `std::array<T, N>`.
 *
 * @tparam T element type
 * @tparam N `std::size_t` giving array size
 *
 * @param from `const std::array<T, N>&` array whose elements we will copy
 */
template <typename T, std::size_t N>
inline Eigen::VectorX<T> eigen_vector_from(const std::array<T, N>& from)
{
  // implicit conversion from unsigned to signed
  Eigen::VectorX<T> to(from.size());
  std::copy(from.cbegin(), from.cend(), to.begin());
  return to;
}

/**
 * Create a new `V_t` *Container* from a list of arguments.
 *
 * Since the template `V_t` argument cannot be deduced, recommended usage is:
 *
 * `auto vec = vector_from<V_t>(arg1, ... argN);`
 *
 * @tparam V_t *Container* type list-initializable by initializer list
 * @tparam Ts parameter pack
 *
 * @param from `const Ts&...` list of arguments castable to `V_t::value_type`
 */
template <typename V_t, typename... Ts>
inline V_t vector_from(const Ts&... from)
{
  return V_t{{static_cast<typename V_t::value_type>(from)...}};
}

/**
 * Create a new `VOut_t` *Container* from another `VIn_t` *Container*.
 *
 * Since the `VOut_t` argument cannot be deduced, recommended usage is:
 *
 * `auto vec = vector_from<VOut_t>(from);`
 *
 * @tparam VOut_t *Container* type
 * @tparam VIn_t *Container* type
 *
 * @param from `const VIn_t&` *Container* to copy elements from
 */
template <typename VOut_t, typename VIn_t>
inline VOut_t vector_from(const VIn_t& from)
{
  // supports std::array, which has a fixed size
  VOut_t to;
  // re-allocate vector types or Container types that have this constructor
  if constexpr (is_vector_size_constructible_v<VOut_t>) {
// MSVC complains about signed to unsigned conversion if using Eigen vectors
#ifdef _MSC_VER
PDMATH_WARNINGS_PUSH()
PDMATH_WARNINGS_DISABLE(4365)
#endif  // _MSC_VER
    to = VOut_t(from.size());
#ifdef _MSC_VER
PDMATH_WARNINGS_POP()
#endif // _MSC_VER
  }
  std::copy(from.cbegin(), from.cend(), to.begin());
  return to;
}

/**
 * Return a `unique_ptr` to a `V_t` *Container* from a list of arguments.
 *
 * Since the template `V_t` argument cannot be deduced, recommended usage is:
 *
 * `auto vec = unique_vector_from<V_t>(arg1, ... argN);`
 *
 * @tparam V_t *Container* type list-initializable by initializer list
 * @tparam Ts parameter pack
 *
 * @param from `const Ts&...` list of arguments castable to `V_t::value_type`
 */
template <typename V_t, typename... Ts>
inline std::unique_ptr<V_t> unique_vector_from(const Ts&... from)
{
  return std::unique_ptr<V_t>(
    new V_t{{static_cast<typename V_t::value_type>(from)...}}
  );
}

/**
 * Return a `unique_ptr` to a *Container* from another `VIn_t` *Container*.
 *
 * Since the `VOut_t` argument cannot be deduced, recommended usage is:
 *
 * `auto vec = unique_vector_from<VOut_t>(from);`
 *
 * @tparam VOut_t *Container* type
 * @tparam VIn_t *Container* type
 *
 *  @param from `const VIn_t&` *Container* to copy elements from
 */
template <typename VOut_t, typename VIn_t>
inline std::unique_ptr<VOut_t> unique_vector_from(const VIn_t& from)
{
  auto to = std::make_unique<VOut_t>();
  // re-allocate vector types or Container types that have this constructor
  if constexpr (is_vector_size_constructible_v<VOut_t>) {
    to = std::make_unique<VOut_t>(from.size());
  }
  std::copy(from.cbegin(), from.cend(), to->begin());
  return to;
}

/**
 * Convenience macro for type aliasing often useful in *Container* wrappers.
 *
 * For a *Container* type `C_t`, provides type aliases for `C_t` as
 * `container_type` and `typename C_t::value_type` as `element_type`.
 *
 * Below we show an example of how to use `PDMATH_USING_CONTAINER_TYPES`.
 *
 * @code{.cc}
 * template <typename V_t>
 * class my_container_wrapper {
 * public:
 *   PDMATH_USING_CONTAINER_TYPES(V_t);
 *   my_container_wrapper(const V_t& value) : value_(value) {}
 *   const V_t& value() const { return value_; }
 * private:
 *   V_t value_;
 * };
 * @endcode
 *
 * @param C_t *Container* type taken by a templated wrapper class
 */
#define PDMATH_USING_CONTAINER_TYPES(C_t) \
  using container_type = C_t; \
  using gradient_type = C_t; \
  using element_type = typename C_t::value_type; \
  using scalar_type = element_type; \
  using value_type = element_type

/**
 * Convenience macro for type aliasing useful in mathematical functor objects.
 *
 * It is most useful for functors that take *Container* types for a gradient
 * and Hessian, with the same `value_type`. The macro also contains a
 * `static_assert` to check that the `value_type` types are the same.
 *
 * Below we show an example of how to use `PDMATH_USING_FUNCTOR_TYPES`.
 *
 * @code{.cc}
 * template <typename G_t, typename H_t>
 * class my_functor {
 * public:
 *   PDMATH_USING_FUNCTOR_TYPES(G_t, H_t);
 *   using gradient_function_t = std::function<G_t(const G_t&)>;
 *   using hessian_function_t = std::function<H_t(const G_t&)>;
 *
 *   my_functor(const gradient_function_t& grad, const hessian_function_t& hess)
 *     : grad_(grad), hess_(hess)
 *   {}
 *
 *   scalar_type operator()(const G_t& x)
 *   {
 *     return std::accumulate(x.cbegin(), x.cend(), 0);
 *   }
 *
 *   const gradient_function_t& grad() const { return grad_; }
 *   const hessian_function_t& hess() const { return hess_; }
 *
 * private:
 *   gradient_function_t grad_;
 *   hessian_function_t hess_;
 * };
 * @endcode
 *
 * @param G_t *Container* type representing the gradient type
 * @param H_t Hessian matrix type, ex. an `Eigen::Matrix` specialization
 */
#define PDMATH_USING_FUNCTOR_TYPES(G_t, H_t) \
  using scalar_type = typename G_t::value_type; \
  using gradient_type = G_t; \
  using hessian_type = H_t; \
  using value_type = scalar_type; \
  static_assert(std::is_same_v<scalar_type, typename H_t::value_type>)

/**
 * Convenience macro for defining non-templated declared class static members.
 *
 * Precludes duplication of the member type declaration in the definition and
 * saves typing, especially for types with long names. See below for example.
 *
 * @code{.cc}
 * struct my_class {
 *   static ns::very_unwieldy_long_type_name default_ctable_static_member;
 * };
 * PDMATH_STATIC_DEFINE(my_class::default_ctable_static_member);
 * @endcode
 *
 * @param member Name of a static class member
 */
#define PDMATH_STATIC_DEFINE(member) decltype(member) member

}  // namespace pdmath

#endif  // PDMATH_HELPERS_H_
