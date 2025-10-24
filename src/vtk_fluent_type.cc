/**
 * @file vtk_fluent_type.cc
 * @author Derek Huang
 * @brief C++ program demo of the VTK helpers' pseudo-expression template types
 * @copyright MIT License
 */

#include <cstdlib>
#include <iostream>
#include <type_traits>

#include <vtkChart.h>
#include <vtkChartXY.h>
#include <vtkContextActor.h>

#include "pdmath/common.h"
#include "pdmath/demangle.h"
#include "pdmath/vtk_actor.h"
#include "pdmath/vtk_chart.h"
#include "pdmath/vtk_window.h"
#include "pdmath/vtk_renderer.h"

namespace {

/**
 * Type wrapper.
 *
 * We only use this to capture the template type parameter.
 *
 * @tparam T type
 */
template <typename T>
struct type_wrapper {
  using type = T;
};

/**
 * Write the type `T` of the `type_wrapper<T>` to the given stream.
 *
 * @todo Replace this with a compile-time type deduction mechanism.
 *
 * @tparam T type
 *
 * @param out Output stream
 */
template <typename T>
auto& operator<<(std::ostream& out, const type_wrapper<T>&)
{
  // typeid() strips cv-ref qualifiers so add back const + volatile
  constexpr auto maybe_const = []
  {
    if constexpr (std::is_const_v<std::remove_reference_t<T>>)
      return "const ";
    else
      return "";
  }();
  constexpr auto maybe_volatile = []
  {
    if constexpr (std::is_volatile_v<std::remove_reference_t<T>>)
      return "volatile ";
    else
      return "";
  }();
  // typeid() strips cv-ref qualifiers so add back reference
  constexpr auto maybe_ref = []
  {
    if constexpr (std::is_reference_v<T>)
      return "&";
    else
      return "";
  }();
  // stream type with qualifiers
  return out << maybe_const << maybe_volatile << PDMATH_DEMANGLE(T) << maybe_ref;
}

/**
 * Print an expression and the type it deduces to.
 *
 * For example, given `1 + 2`, `"1 + 2\n  -> int"` would be printed.
 *
 * @param expr Expression to print `decltype()` for
 */
#define PRINT_DECLTYPE(expr) \
  std::cout << PDMATH_STRINGIFY(expr) << \
    "\n  -> " << type_wrapper<decltype(expr)>() << std::endl

}  // namespace

int main()
{
#define EXPR_1 pdmath::vtk_window{}
  PRINT_DECLTYPE(EXPR_1);
#define EXPR_2 EXPR_1.renderer()
  PRINT_DECLTYPE(EXPR_2);
#define EXPR_3 EXPR_2.add<vtkContextActor>()
  PRINT_DECLTYPE(EXPR_3);
#define EXPR_4 EXPR_3.scene()
  PRINT_DECLTYPE(EXPR_4);
#define EXPR_5 EXPR_4.add<vtkChartXY>()
  PRINT_DECLTYPE(EXPR_5);
#define EXPR_6 EXPR_5.plot<vtkChart::LINE>()
  PRINT_DECLTYPE(EXPR_6);
#define EXPR_7 EXPR_6.width(3.)
  PRINT_DECLTYPE(EXPR_7);
#define EXPR_8 EXPR_7()
  PRINT_DECLTYPE(EXPR_8);
  return EXIT_SUCCESS;
}
