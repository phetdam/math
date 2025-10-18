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
 * // VTK colors object + convenience lambda for named RGBA color
 * vtkNew<vtkColors> colors;
 * auto named_color = [&colors](auto name) { return colors->GetColor4ub(name); };
 * // create the chart with sin, cos, tan plots
 * auto chart = vtk_xy_chart{}
 *   // set up title, background color, and background color opacity
 *   .title("sin, cos, tan")
 *   .color(named_color("Thistle"))
 *   .opacity(0.5)
 *   // update x (bottom) and y (left) axis grid line colors + titles
 *   .axis(vtkAxis::BOTTOM)
 *     .grid_color(named_color("LightCyan"))
 *     .title("x")
 *   ()
 *   .axis(vtkAxis::LEFT)
 *     .grid_color(named_color("LightCyan"))
 *     .title("y")
 *   ()
 *   // add sin(x), cos(x), and tan(x) line plots
 *   .plot<vtkChart::LINE>()
 *     .data(table, "x", "sin(x)")
 *     .color(named_color("Red"))
 *     .width(3.)
 *     .label("sin(x)")
 *   ()
 *   .plot<vtkChart::LINE>()
 *     .data(table, "x", "cos(x)")
 *     .color(named_color("CornflowerBlue"))
 *     .width(3.)
 *     .label("cos(x)")
 *   ()
 *   .plot<vtkChart::LINE>()
 *     .data(table, "x", "tan(x)")
 *     .color(named_color("Green"))
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
 */
template <typename T = void>
class vtk_xy_chart {
public:
  // indicate if owning or not
  static constexpr bool owning = std::is_void_v<T>;
  // TODO: finish prototyping for compile-time ownership architecture
  // parent type check (void for standalone type)
  static_assert(
    owning ||
    (!std::is_pointer_v<T> && std::is_convertible_v<T, vtkContextScene*>)
  );

  using parent_type = T;
  using axis_type = vtk_axis<vtk_xy_chart>;
  using legend_type = vtk_legend<vtk_xy_chart>;

  template <int V>
  using plot_type = vtk_plot<V, vtk_xy_chart>;

  /**
   * Default ctor.
   *
   * This is only available when the `vtk_xy_chart` has no parent.
   *
   * @note Extra template arg is used to make the declaration dependent on a
   *  template type and therefore delay instantiation and use SFINAE.
   */
  template <typename U = int>
  vtk_xy_chart(std::enable_if_t<owning, U> = 0) : parent_{} {}

  template <typename U = int>
  vtk_xy_chart(
    vtkChartXY* chart,
    T* parent = nullptr,
    std::enable_if_t<!owning, U> = 0) noexcept
    : chart_{chart}, parent_{parent}
  {}

  // TODO: document more

  auto operator->() const noexcept
  {
    if constexpr (owning)
      return chart_.Get();
    else
      return chart_;
  }

  auto&& operator()()
  {
    if constexpr (owning)
      return std::move(*this);
    else
      return *parent_;
  }

  operator vtkChartXY*() const noexcept
  {
    return chart_;
  }

  auto axis(vtkAxis::Location loc)
  {
    return axis_type{chart_->GetAxis(loc), this};
  }

  // background color
  auto& color(const vtkColor4ub& color)
  {
    chart_->GetBackgroundBrush()->SetColor(color);
    return *this;
  }

  auto color() const
  {
    return chart_->GetBackgroundBrush()->GetColorObject();
  }

  auto& opacity(double v)
  {
    chart_->GetBackgroundBrush()->SetOpacityF(v);
    return *this;
  }

  // create a *new* plot
  template <int V>
  auto plot()
  {
    return plot_type<V>{chart_->AddPlot(V), this};
  }

  auto& title(const vtkStdString& text)
  {
    chart_->SetTitle(text);
    return *this;
  }

  auto title() const
  {
    return chart_->GetTitle();
  }

  auto legend()
  {
    return legend_type{chart_->GetLegend(), this};
  }

  // version that also sets whether or not the legend is visible
  auto legend(bool show)
  {
    chart_->SetShowLegend(show);
    return legend();
  }

private:
  std::conditional_t<owning, vtkNew<vtkChartXY>, vtkChartXY*> chart_;
  parent_type* parent_;
};

namespace v2 {

/**
 * `vtkChartXY` wrapper with fluent API.
 *
 * This is the `v2` implementation using `vtk_skeleton`.
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

}  // namespace v2

}  // namespace pdmath

#endif  // PDMATH_VTK_CHART_H_
