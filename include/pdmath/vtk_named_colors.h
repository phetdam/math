/**
 * @file vtk_named_colors.h
 * @author Derek Huang
 * @brief C++ header for `vtkNamedColors` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_NAMED_COLORS_H_
#define PDMATH_VTK_NAMED_COLORS_H_

#include <cstddef>

#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkStdString.h>

#include "pdmath/common.h"

namespace pdmath {

/**
 * Define a `vtkStdString` subclass to distinguish a particular color format.
 *
 * @param suffix VTK color suffix, e.g. `4ub`, `3d`
 */
#define PDMATH_VTK_COLOR_NAME_TYPE(suffix) \
  class PDMATH_CONCAT(vtk_color_name_, suffix) : public vtkStdString { \
  public: \
    using vtkStdString::vtkStdString; \
  }

// define for all VTK color suffixes
PDMATH_VTK_COLOR_NAME_TYPE(3d);
PDMATH_VTK_COLOR_NAME_TYPE(4d);
PDMATH_VTK_COLOR_NAME_TYPE(3ub);
PDMATH_VTK_COLOR_NAME_TYPE(4ub);

/**
 * Namespace for VTK user-defined literals.
 *
 * Bring these into the current scope with `using pdmath::vtk_literals`.
 */
namespace vtk_literals {

/**
 * Define a string literal operator for the given VTK color suffix.
 *
 * @param suffix VTK color suffix, e.g. `4ub`, `3d`
 */
#define PDMATH_VTK_COLOR_NAME_LITERAL(suffix) \
  inline PDMATH_CONCAT(vtk_color_name_, suffix) operator \
  PDMATH_CONCAT("", PDMATH_CONCAT(_, suffix))(const char* s, std::size_t n) \
  { \
    return {s, n}; \
  } \
  /* force semicolon usage */ \
  static_assert(true)

// define for all VTK color suffixes
PDMATH_VTK_COLOR_NAME_LITERAL(3d);
PDMATH_VTK_COLOR_NAME_LITERAL(4d);
PDMATH_VTK_COLOR_NAME_LITERAL(3ub);
PDMATH_VTK_COLOR_NAME_LITERAL(4ub);

}  // namespace vtk_literals

/**
 * `vtkNamedColors` functional wrapper.
 *
 * This simplifies the use of the `vtkNamedColors` object which has a verbose
 * API and doesn't provide a short and concise way of retrieving colors.
 *
 * For example, the standard way of using `vtkNamedColors`:
 *
 * @code{.cc}
 * vtkNew<vtkNamedColors> nc;
 * auto lc_4ub = nc->GetColor4ub("LightCyan");
 * auto lc_3d = nc->GetColor3d("LightCyan");
 * @endcode
 *
 * The `vtk_named_colors` provides a more modern interface:
 *
 * @code{.cc}
 * using namespace pdmath::vtk_literals;
 * vtk_named_colors nc;
 * auto lc_4ub = nc("LightCyan"_4ub);
 * auto lc_3d = nc("LightCyan"_3d);
 * @endcode
 */
class vtk_named_colors {
public:
/**
 * Define `operator()` for `vtk_named_colors` for the VTK color suffix.
 *
 * @param suffix VTK color suffix, e.g. `4ub`, `3d`
 */
#define PDMATH_VTK_NAMED_COLORS_CALL(suffix) \
  auto operator()(const PDMATH_CONCAT(vtk_color_name_, suffix)& name) const \
  { \
    return colors_->PDMATH_CONCAT(GetColor, suffix)(name); \
  } \
  /* force semicolon usage */ \
  static_assert(true)

  // define for all VTK color suffixes
  PDMATH_VTK_NAMED_COLORS_CALL(3d);
  PDMATH_VTK_NAMED_COLORS_CALL(4d);
  PDMATH_VTK_NAMED_COLORS_CALL(3ub);
  PDMATH_VTK_NAMED_COLORS_CALL(4ub);

  /**
   * Provide access to the `vtkNamedColors` members.
   */
  auto operator->() const noexcept
  {
    return colors_.Get();
  }

private:
  vtkNew<vtkNamedColors> colors_;
};

}  // namespace pdmath

#endif  // PDMATH_VTK_NAMED_COLORS_H_
