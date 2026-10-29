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
   * Class representing a real interval.
   *
   * This is used to represent the minimum and maximum of an axis.
   */
  class interval {
  public:
    /**
     * Ctor.
     *
     * @param lower Lower endpoint
     * @param upper Upper endpoint
     */
    interval(double lower, double upper) noexcept
      : lower_{lower}, upper_{upper}
    {}

    /**
     * Return the lower endpoint.
     */
    auto lower() const noexcept { return lower_; }

    /**
     * Return the upper endpoint.
     */
    auto upper() const noexcept { return upper_; }

  private:
    double lower_;
    double upper_;
  };

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
   * Set the logical minimum value of the axis in plot coordinates.
   *
   * @note To render with the set range remember to also call `fixed()`.
   *
   * @param lo Minimum axis value
   */
  auto& lower(double lo)
  {
    object()->SetMinimum(lo);
    return *this;
  }

  /**
   * Return the logical minimum value of the axis in plot coordinates.
   */
  auto lower() const
  {
    return object()->GetMinimum();
  }

  /**
   * Set the logical maximum value of the axis in plot coordinates.
   *
   * @note To render with the set range remember to also call `fixed()`.
   *
   * @param hi Maximum axis value
   */
  auto& upper(double hi)
  {
    object()->SetMaximum(hi);
    return *this;
  }

  /**
   * Return the logical maximum value of the axis in plot coordinates.
   */
  auto upper() const
  {
    return object()->GetMaximum();
  }

  /**
   * Set the axis behavior.
   *
   * By default, the axis auto-scales, so calling `lower()`, `upper()`, or
   * `range()` will not have any effect. Ensure that this member is called with
   * `vtkAxis::FIXED` before rendering so the axis limits are respected.
   *
   * @param kind Axis behavior, e.g. `vtkAxis::FIXED` or `vtkAxis::AUTO`
   */
  auto& behavior(int kind)
  {
    object()->SetBehavior(kind);
    return *this;
  }

  /**
   * Return the axis behavior.
   */
  auto behavior() const
  {
    return object()->GetBehavior();
  }

  /**
   * Set the logical range of the axis in plot coordinates.
   *
   * @note To render with the set range remember to also call `fixed()` or use
   *  the overload of `fixed()` that also calls `range()`.
   *
   * @param lo Minimum axis value
   * @param hi Maximum axis value
   */
  auto& range(double lo, double hi)
  {
    object()->SetRange(lo, hi);
    return *this;
  }

  /**
   * Return the logical range of the axis in plot coordinates.
   */
  auto range() const
  {
    double bounds[2];
    object()->GetRange(bounds);
    return interval{bounds[0], bounds[1]};
  }

  /**
   * Set the axis behavior as fixed.
   *
   * A fixed axis respects custom minimum and maximum axis values.
   */
  auto& fixed()
  {
    return behavior(vtkAxis::FIXED);
  }

  /**
   * Set the axis behavior as fixed and set the axis range.
   *
   * This ensures that the fixed axis range is rendered correctly.
   *
   * @param lo Minimum axis value
   * @param hi Maximum axis value
   */
  auto& fixed(double lo, double hi)
  {
    fixed();
    return range(lo, hi);
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
