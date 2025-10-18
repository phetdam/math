/**
 * @file vtk_chart.h
 * @author Derek Huang
 * @brief C++ header for `vtkChart` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_CHART_H_
#define PDMATH_VTK_CHART_H_

#include <type_traits>
#include <utility>

#include <vtkAxis.h>
#include <vtkBrush.h>
#include <vtkChart.h>
#include <vtkChartXY.h>
#include <vtkColor.h>
#include <vtkChartLegend.h>
#include <vtkNew.h>
#include <vtkPen.h>
#include <vtkStdString.h>
#include <vtkTable.h>

#include "pdmath/type_traits.h"
#include "pdmath/vtk_axis.h"
#include "pdmath/vtk_legend.h"
#include "pdmath/vtk_plot.h"
#include "pdmath/vtk_skeleton.h"

namespace pdmath {

/**
 * `vtkChartXY` wrapper with fluent API.
 *
 * This simplifies the creation of a single `vtkChartXY` and the associated
 * steps of updating its labels, colors, axes, and adding new plots. The fluent
 * API cuts down on the verbosity of the standard VTK getter/setter API and
 * some template member functions help prevent use of invalid enums.
 *
 * For example, given the table data in the `vtk_table` example, we could
 * easily create a `vtkChartXY` with line plots for each data column:
 *
 * @code{.cc}
 * // note: how table is defined can be found in the vtk_table example
 * // note: see vtk_named_colors.h for details on using vtk_named_colors
 * using namespace pdmath::vtk_literals;
 * vtk_named_colors nc;
 * // create the chart with sin, cos, tan plots
 * auto chart = vtk_xy_chart{}
 *   // set up title, background color, and background color opacity
 *   .title("sin, cos, tan")
 *   .color(nc("Thistle"_4ub))
 *   .opacity(0.5)
 *   // update x (bottom) and y (left) axis grid line colors + titles
 *   .axis(vtkAxis::BOTTOM)
 *     .grid_color(nc("LightCyan"_4ub))
 *     .title("x")
 *   ()
 *   .axis(vtkAxis::LEFT)
 *     .grid_color(nc("LightCyan"_4ub))
 *     .title("y")
 *   ()
 *   // add sin(x), cos(x), and tan(x) line plots
 *   .plot<vtkChart::LINE>()
 *     .data(table, "x", "sin(x)")
 *     .color(nc("Red"_4ub))
 *     .width(3.)
 *     .label("sin(x)")
 *   ()
 *   .plot<vtkChart::LINE>()
 *     .data(table, "x", "cos(x)")
 *     .color(nc("CornflowerBlue"_4ub))
 *     .width(3.)
 *     .label("cos(x)")
 *   ()
 *   .plot<vtkChart::LINE>()
 *     .data(table, "x", "tan(x)")
 *     .color(nc("Green"_4ub))
 *     .width(3.)
 *     .label("tan(x)")
 *   ()
 *   // add visible legend aligned to top left so it doesn't cover tan(x)
 *   .legend()
 *     .align<vtkChartLegend::LEFT, vtkChartLegend::TOP>()
 *     .show()
 *   ()
 *   ();
 * @endcode
 *
 * @tparam T Parent type
 */
template <typename T = void>
class vtk_xy_chart : public vtk_skeleton<vtk_xy_chart<T>, vtkChartXY, T> {
public:
  PDMATH_USING_VTK_SKELETON(vtk_xy_chart<T>, vtkChartXY, T);
  using axis_type = vtk_axis<vtk_xy_chart>;
  using legend_type = vtk_legend<vtk_xy_chart>;

  // TODO: document more and replace original implementation

  template <int V>
  using plot_type = vtk_plot<V, vtk_xy_chart>;

  auto axis(vtkAxis::Location loc)
  {
    return axis_type{object()->GetAxis(loc), this};
  }

  // background color
  auto& color(const vtkColor4ub& color)
  {
    object()->GetBackgroundBrush()->SetColor(color);
    return *this;
  }

  auto color() const
  {
    return object()->GetBackgroundBrush()->GetColorObject();
  }

  auto& opacity(double v)
  {
    object()->GetBackgroundBrush()->SetOpacityF(v);
    return *this;
  }

  // create a *new* plot
  template <int V>
  auto plot()
  {
    return plot_type<V>{object()->AddPlot(V), this};
  }

  auto& title(const vtkStdString& text)
  {
    object()->SetTitle(text);
    return *this;
  }

  auto title() const
  {
    return object()->GetTitle();
  }

  auto legend()
  {
    return legend_type{object()->GetLegend(), this};
  }

  // version that also sets whether or not the legend is visible
  auto legend(bool show)
  {
    object()->SetShowLegend(show);
    return legend();
  }
};

}  // namespace pdmath

#endif  // PDMATH_VTK_CHART_H_
