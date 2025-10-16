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
#include <vtkPlotPoints.h>
#include <vtkStdString.h>
#include <vtkTable.h>

#include "pdmath/type_traits.h"

namespace pdmath {

/**
 * Template class representing a `vtkAxis` owned by another object.
 *
 * This acts as a cheap proxy object for modifying axis settings before
 * returning back to the parent object's fluent API.
 *
 * The parent type could be `vtk_chart` or any type convertible to `vtkChart*`.
 *
 * @tparam T Parent type convertible to `vtkChart*`
 */
template <typename T>
class vtk_child_axis {
public:
  static_assert(std::is_convertible_v<T, vtkChart*>);  // check
  using parent_type = T;

  /**
   * Ctor.
   *
   * @param axis Axis object pointer
   * @param parent Parent object pointer
   */
  vtk_child_axis(vtkAxis* axis, parent_type* parent = nullptr) noexcept
    : axis_{axis}, parent_{parent}
  {}

  /**
   * Provide member access to the pointed-to `vtkAxis`.
   */
  auto operator->() const noexcept
  {
    return axis_;
  }

  /**
   * Return reference to parent object to support method chaining.
   *
   * Like the `operator()` provided by the `vtk_chart`, this marks the "end" of
   * settings update for the axis, but has the additional effect of indicating
   * return back the parent (`vtkChart` typically) object's scope.
   */
  auto& operator()() noexcept
  {
    return *parent_;
  }

  /**
   * Get the RGBA axis grid color object.
   */
  auto grid_color() const
  {
    return axis_->GetGridPen()->GetColorObject();
  }

  /**
   * Set the axis grid color.
   *
   * @param color RGBA color to to use for the axis grid
   */
  auto& grid_color(const vtkColor4ub& color)
  {
    axis_->GetGridPen()->SetColor(color);
    return *this;
  }

  /**
   * Get the axis title string.
   */
  auto title() const
  {
    return axis_->GetTitle();
  }

  /**
   * Set the axis title string.
   *
   * @param text Axis title text
   */
  auto& title(const vtkStdString& text)
  {
    axis_->SetTitle(text);
    return *this;
  }

private:
  vtkAxis* axis_;
  parent_type* parent_;
};

/**
 * Template class representing a `vtkPlot` owned by another object.
 *
 * This is a cheap proxy object for modifying plot settings before returning
 * back to the parent object's fluent API. The parent type could be `vtk_chart`
 * or any type convertible to `vtkChart*`.
 *
 * @tparam T Parent type convertible to `vtkChart*`
 * @tparam V Plot type, e.g. `vtkChart::LINE`, etc.
 */
template <typename T, int V>
class vtk_child_plot {
public:
  static_assert(std::is_convertible_v<T, vtkChart*>);  // check
  using parent_type = T;

  // plot type from the vtkChart plot type constants, e.g. vtkChart::LINE
  static constexpr auto type = V;

  /**
   * Ctor.
   *
   * @param plot Plot object pointer
   * @param parent Parent object pointer
   */
  vtk_child_plot(vtkPlot* plot, parent_type* parent = nullptr) noexcept
    : plot_{plot}, parent_{parent}
  {}

  /**
   * Provide member access to the pointed-to `vtkPlot`.
   */
  auto operator->() const noexcept
  {
    return plot_;
  }

  /**
   * Return reference to parent object to support method chaining.
   *
   * @todo Maybe put this and `operator->` in a template mixin.
   */
  auto& operator()()
  {
    return *parent_;
  }

  /**
   * Return the RGB plotting color.
   *
   * This is the color used to draw the plot, e.g. a plot line, points, etc.
   *
   * @note One can set an alpha value for the plot color (RGBA).
   */
  auto color() const
  {
    unsigned char rgb[3];
    plot_->GetColor(rgb);
    return vtkColor3ub{rgb};
  }

  /**
   * Update the RGBA plot color.
   *
   * @note The alpha value cannot be retrieved with the `color()` getter.
   *
   * @param color RGBA color to use when drawing the plot points, lines, etc.
   */
  auto& color(const vtkColor4ub& color)
  {
    plot_->SetColor(
      color.GetRed(),
      color.GetGreen(),
      color.GetBlue(),
      color.GetAlpha()
    );
    return *this;
  }

  /**
   * Update the x, y input data used for the plot by column index.
   *
   * @param table Table containing the plot data
   * @param xcol Column index for x values
   * @param ycol Column index for y values
   */
  auto& data(vtkTable* table, vtkIdType xcol, vtkIdType ycol)
  {
    plot_->SetInputData(table, xcol, ycol);
    return *this;
  }

  /**
   * Update the x, y input data used for the plot by column name.
   *
   * @param table Table containing the plot data
   * @param xcol Column name for x values
   * @param ycol Column name for y values
   */
  auto& data(vtkTable* table, const vtkStdString& xcol, const vtkStdString& ycol)
  {
    plot_->SetInputData(table, xcol, ycol);
    return *this;
  }

  /**
   * Return the plot label used in the chart legend as a string.
   */
  auto label() const
  {
    return plot_->GetLabel();
  }

  /**
   * Update the plot label used in the chart legend.
   *
   * @param text Label text
   */
  auto& label(const vtkStdString& text)
  {
    plot_->SetLabel(text);
    return *this;
  }

  /**
   * Update the marker style used by the plot.
   *
   * This member function is conditionally available for point/line plots.
   *
   * @param style Plot marker style, e.g. `vtkPlotPoints::PLUS`
   */
  template <constraint_t<vtkChart::POINTS == V || vtkChart::LINE == V> = 0>
  auto& marker(int style)
  {
    // vtkPlotLine inherits vtkPlotPoints so casting to base is enough
    dynamic_cast<vtkPlotPoints*>(plot_)->SetMarkerStyle(style);
    return *this;
  }

  /**
   * Get the width of the plotting line.
   *
   * @note Not sure what the units are but they appear to be in fractional em.
   *
   * This does not affect `vtkChart::POINTS` plots that don't draw plot lines.
   */
  auto width() const
  {
    return plot_->GetWidth();
  }

  /**
   * Update the width of the plotting line.
   *
   * @note Not sure what the units are but they appear to be in fractional em.
   *
   * @param em Line width
   */
  auto& width(float em)
  {
    plot_->SetWidth(em);
    return *this;
  }

private:
  vtkPlot* plot_;
  parent_type* parent_;
};

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
 * @tparam T Parent type convertible to `vtkChart*`
 */
template <typename T>
class vtk_child_legend {
public:
  static_assert(std::is_convertible_v<T, vtkChart*>);  // check
  using parent_type = T;

  /**
   * Ctor.
   *
   * @param legend Chart legend pointer
   * @param parent Parent object pointer
   */
  vtk_child_legend(vtkChartLegend* legend, parent_type* parent = nullptr) noexcept
    : legend_{legend}, parent_{parent}
  {}

  /**
   * Provide member access to the pointed-to `vtkChartLegend`.
   */
  auto operator->() const noexcept
  {
    return legend_;
  }

  /**
   * Return reference to parent object to support method chaining.
   *
   * @todo Maybe put this and `operator->` in a template mixin.
   */
  auto& operator()()
  {
    return *parent_;
  }

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
      legend_->SetHorizontalAlignment(v);
    // y alignment value
    if constexpr (y_align_v<v>)
      legend_->SetVerticalAlignment(v);
    return *this;
  }

  /**
   * Return the horizontal and vertical alignment of the legend.
   */
  auto align() const
  {
    return std::pair{
      legend_->GetHorizontalAlignment(),
      legend_->GetVerticalAlignment()
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
      legend_->SetHorizontalAlignment(x);
    if constexpr (y != vtk_no_align)
      legend_->SetVerticalAlignment(y);
    return *this;
  }

  /**
   * Indicate that the parent should draw the legend.
   *
   * @param v `true` to show the legend, `false` to hide
   */
  auto& show(bool v = true)
  {
    // note: static_assert already checked that *parent_ converts to vtkChart*
    static_cast<vtkChart*>(*parent_)->SetShowLegend(v);
    return *this;
  }

private:
  vtkChartLegend* legend_;
  parent_type* parent_;
};

struct vtk_no_parent {};

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
private:
  // TODO: finish prototyping for compile-time ownership architecture
  // parent type check (void for standalone type)
  static_assert(
    std::is_same_v<T, void> ||
    (!std::is_pointer_v<T> && std::is_convertible_v<T, vtkContextScene*>) ||
    std::is_base_of_v<vtkContextScene, T>
  );
  // member type
  using chart_ptr = std::conditional_t<
    std::is_void_v<T>,
    vtkNew<vtkChartXY>,  // owning
    vtkChartXY*          // non-owning
  >;

public:
  using parent_type = T;
  using axis_type = vtk_child_axis<vtk_xy_chart>;
  using legend_type = vtk_child_legend<vtk_xy_chart>;

  /**
   * Default ctor.
   *
   * This is only available when the `vtk_xy_chart` has no parent.
   *
   * @note Extra template arg is used to make the declaration dependent on a
   *  template type and therefore delay instantiation and use SFINAE.
   */
  template <typename U = int>
  vtk_xy_chart(std::enable_if_t<std::is_void_v<T>, U> = 0) : parent_{} {}

  template <typename U = int>
  vtk_xy_chart(
    vtkChartXY* chart,
    T* parent = nullptr,
    std::enable_if_t<!std::is_void_v<T>, U> = 0) noexcept
    : chart_{chart}, parent_{parent}
  {}

  // TODO: document more

  template <int V>
  using plot_type = vtk_child_plot<vtk_xy_chart, V>;

  auto operator->() const noexcept
  {
    if constexpr (std::is_void_v<T>)
      return chart_.Get();
    else
      return chart_;
  }

  auto&& operator()()
  {
    if constexpr (std::is_void_v<T>)
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
  chart_ptr chart_;
  parent_type* parent_;
};

}  // namespace pdmath

#endif  // PDMATH_VTK_CHART_H_
