/**
 * @file vtk_axis.h
 * @author Derek Huang
 * @brief C++ header for `vtkAxis` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_AXIS_H_
#define PDMATH_VTK_AXIS_H_

#include <vtkAxis.h>

#include "pdmath/vtk_skeleton.h"

namespace pdmath {

/**
 * `vtkAxis` wrapper with fluent API.
 *
 * This provides a fluent API for the `vtkAxis` while using the `vtk_skeleton`
 * ownership architecture to model ownership.
 *
 * @tparam T Parent type
 */
template <typename T = void>
class vtk_axis : public vtk_skeleton<vtk_axis<T>, vtkAxis, T> {
public:
  PDMATH_USING_VTK_SKELETON(vtk_axis<T>, vtkAxis, T);

  /**
   * Get the RGBA axis grid color object.
   */
  auto grid_color() const
  {
    return object()->GetGridPen()->GetColorObject();
  }

  /**
   * Set the axis grid color.
   *
   * @param color RGBA color to to use for the axis grid
   */
  auto& grid_color(const vtkColor4ub& color)
  {
    object()->GetGridPen()->SetColor(color);
    return *this;
  }

  /**
   * Get the axis title string.
   */
  auto title() const
  {
    return object()->GetTitle();
  }

  /**
   * Set the axis title string.
   *
   * @param text Axis title text
   */
  auto& title(const vtkStdString& text)
  {
    object()->SetTitle(text);
    return *this;
  }
};

}  // namespace pdmath

#endif  // PDMATH_VTK_AXIS_H_
