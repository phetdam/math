/**
 * @file vtk_axis.h
 * @author Derek Huang
 * @brief C++ header for `vtkAxis` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_AXIS_H_
#define PDMATH_VTK_AXIS_H_

#include <vtkAxis.h>
#include <vtkPen.h>

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
   * Indicate if the grid lines are visible or not.
   */
  bool grid_visible() const
  {
    return object()->GetGridVisible();
  }

  /**
   * Indicate if the grid lines should be visible or not.
   *
   * @param v `true` for grid lines to be visible, `false` to hide
   */
  auto& grid_visible(bool v)
  {
    object()->SetGridVisible(v);
    return *this;
  }

  /**
   * Hide the grid lines.
   */
  auto& hide_grid()
  {
    return grid_visible(false);
  }

  /**
   * Show the grid lines.
   */
  auto& show_grid()
  {
    return grid_visible(true);
  }

  /**
   * Get the axis data label offset in fractional pixels from the axis.
   */
  auto label_offset() const
  {
    return object()->GetLabelOffset();
  }

  /**
   * Set the axis data lavel offset in fractional pixels from the axis.
   *
   * @param px Number of pixels separating the axis label from the axis
   */
  auto& label_offset(float px)
  {
    object()->SetLabelOffset(px);
    return *this;
  }

  /**
   * Indicate if the axis data labels are visible or not.
   */
  bool labels_visible() const
  {
    return object()->GetLabelsVisible();
  }

  /**
   * Indicage if the axis data labels should be visible or not.
   *
   * @param v `true` to show the axis data labels, `false` to hide
   */
  auto& labels_visible(bool v)
  {
    object()->SetLabelsVisible(v);
    return *this;
  }

  /**
   * Hide the axis data labels.
   */
  auto& hide_labels()
  {
    return labels_visible(false);
  }

  /**
   * Show the axis data labels.
   */
  auto& show_labels()
  {
    return labels_visible(true);
  }

  /**
   * Get the axis tick length in fractional pixels.
   */
  auto tick_length() const
  {
    return object()->GetTickLength();
  }

  /**
   * Set the axis tick length in fractional pixels.
   *
   * @param len Tick length in fractional pixels
   */
  auto& tick_length(float len)
  {
    return object()->SetTickLength(len);
    return *this;
  }

  /**
   * Get number of tick marks for the axis.
   */
  auto ticks() const
  {
    return object()->GetNumberOfTicks();
  }

  /**
   * Set the number of tick marks for the axis.
   *
   * If set to `-1` the tick mark calculation is done automatically.
   *
   * @param n Number of ticks to mark on the axis or `-1`
   */
  auto& ticks(int n)
  {
    object()->SetNumberOfTicks(n);
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

  /**
   * Indicate if the axis title is visible.
   */
  bool title_visible() const
  {
    return object()->GetTitleVisible();
  }

  /**
   * Indicate if the axis title should be visible.
   *
   * @param v `true` to show axis title, `false` to hide
   */
  auto& title_visible(bool v)
  {
    object()->SetTitleVisible(v);
    return *this;
  }

  /**
   * Hide the axis title.
   */
  auto& hide_title()
  {
    return title_visible(false);
  }

  /**
   * Show the axis title.
   */
  auto& show_title()
  {
    return title_visible(true);
  }
};

// deduction guide for MSVC
vtk_axis() -> vtk_axis<>;

/**
 * Partial specialization for `vtk_skeleton_type`.
 *
 * @tparam P Parent type
 */
template <typename P>
struct vtk_skeleton_type<vtkAxis, P> {
  using type = vtk_axis<P>;
};

}  // namespace pdmath

#endif  // PDMATH_VTK_AXIS_H_
