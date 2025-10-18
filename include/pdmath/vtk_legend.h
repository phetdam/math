/**
 * @file vtk_legend.h
 * @author Derek Huang
 * @brief C++ header for `vtkChartLegend` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_LEGEND_H_
#define PDMATH_VTK_LEGEND_H_

#include <vtkChart.h>
#include <vtkChartLegend.h>

#include "pdmath/type_traits.h"
#include "pdmath/vtk_skeleton.h"

namespace pdmath {

/**
 * Special constant indicating a legend should not be aligned.
 *
 * @todo May want to move this to a header like `vtk_common.h` later.
 */
inline constexpr int vtk_no_align = -1;

/**
 * Template class representing a `vtkChartLegend` owned by another object.
 *
 * This is a cheap project object for modifying legend settings before
 * returning back to the parent object's fluent API.
 *
 * @tparam T Parent type
 */
template <typename T = void>
class vtk_legend : public vtk_skeleton<vtk_legend<T>, vtkChartLegend, T> {
private:
  /**
   * Traits flag enumerating the valid values for horizontal alignment.
   *
   * @tparam v Horizontal legend alignment value
   */
  template <int v>
  static constexpr bool x_align_v = (
    v == vtkChartLegend::LEFT ||
    v == vtkChartLegend::CENTER ||
    v == vtkChartLegend::RIGHT ||
    v == vtkChartLegend::CUSTOM
  );

  /**
   * Traits flag enumerating the valid values for vertical alignment.
   *
   * @tparam v Vertical legend alignment value
   */
  template <int v>
  static constexpr bool y_align_v = (
    v == vtkChartLegend::TOP ||
    v == vtkChartLegend::CENTER ||
    v == vtkChartLegend::BOTTOM ||
    v == vtkChartLegend::CUSTOM
  );

public:
  PDMATH_USING_VTK_SKELETON(vtk_legend<T>, vtkChartLegend, T);

  /**
   * Update the horizontal or vertical alignment of the legend.
   *
   * If the alignment value is only for horizontal alignment then only the
   * horizontal alignment is updated and the same is true for vertical
   * alignment. However, if the alignment is `vtkChartLegend::CENTER` or
   * `vtkChartLegend::CUSTOM`, values which apply for both horitzontal and
   * vertical alignment, both horizontal and vertical alignment are updated.
   *
   * @tparam v Horizontal or vertical alignment value
   */
  template <int v>
  auto& align(constraint_t<x_align_v<v> || y_align_v<v>> = 0)
  {
    // x alignment value
    if constexpr (x_align_v<v>)
      object()->SetHorizontalAlignment(v);
    // y alignment value
    if constexpr (y_align_v<v>)
      object()->SetVerticalAlignment(v);
    return *this;
  }

  /**
   * Return the horizontal and vertical alignment of the legend.
   */
  auto align() const
  {
    return std::pair{
      object()->GetHorizontalAlignment(),
      object()->GetVerticalAlignment()
    };
  }

  /**
   * Update the horizontal and vertical alignment of the legend.
   *
   * One can specify `vtk_no_align` to set the alignment for only one axis. This is
   * useful when trying to set alignment values like `vtkChartLegend::CENTER`,
   * as `align<vtkChartLegend::CENTER>()` will apply the alignment to both
   * horizontal and vertical axes. By using `vtk_no_align` with this overload, one
   * can set just the vertical alignment using the invocation
   * `align<vtk_no_align, vtkChartLegend::CENTER>()` instead.
   *
   * @tparam x Horizontal alignment value or `vtk_no_align`
   * @tparam y Vertical alignment value or `vtk_no_align`
   */
  template <int x, int y>
  auto& align(
    constraint_t<
      (x_align_v<x> || x == vtk_no_align) &&
      (y_align_v<y> || y == vtk_no_align) > = 0)
  {
    if constexpr (x != vtk_no_align)
      object()->SetHorizontalAlignment(x);
    if constexpr (y != vtk_no_align)
      object()->SetVerticalAlignment(y);
    return *this;
  }

  /**
   * Indicate that the parent should draw the legend.
   *
   * @param v `true` to show the legend, `false` to hide
   */
  auto& show(bool v = true)
  {
    // note: static_assert already checked that *parent() converts to vtkChart*
    static_cast<vtkChart*>(*parent())->SetShowLegend(v);
    return *this;
  }
};

}  // namespace pdmath

#endif  // PDMATH_VTK_LEGEND_H_
