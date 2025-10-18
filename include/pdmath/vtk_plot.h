/**
 * @file vtk_plot.h
 * @author Derek Huang
 * @brief C++ header for `vtkPlot` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_PLOT_H_
#define PDMATH_VTK_PLOT_H_

#include <vtkChart.h>
#include <vtkColor.h>
#include <vtkPlot.h>
#include <vtkPlotPoints.h>
#include <vtkStdString.h>
#include <vtkTable.h>
#include <vtkType.h>

#include "pdmath/type_traits.h"
#include "pdmath/vtk_skeleton.h"

namespace pdmath {

/**
 * `vtkPlot` wrapper with fluent API.
 *
 * This provides a fluent API for the `vtkAxis` while using the `vtk_skeleton`
 * ownership architecture to model ownership. Plot type is determined at
 * compile time via one of the `vtkChart` plot type constants.
 *
 * @tparam V Plot type, e.g. `vtkChart::LINE`, etc.
 * @tparam T Parent type
 */
template <int V, typename T = void>
class vtk_plot : public vtk_skeleton<vtk_plot<V, T>, vtkPlot, T> {
public:
  PDMATH_USING_VTK_SKELETON(vtk_plot<V, T>, vtkPlot, T);
  // plot type from the vtkChart plot type constants, e.g. vtkChart::LINE
  static constexpr auto type = V;

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
    object()->GetColor(rgb);
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
    object()->SetColor(
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
    object()->SetInputData(table, xcol, ycol);
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
    object()->SetInputData(table, xcol, ycol);
    return *this;
  }

  /**
   * Return the plot label used in the chart legend as a string.
   */
  auto label() const
  {
    return object()->GetLabel();
  }

  /**
   * Update the plot label used in the chart legend.
   *
   * @param text Label text
   */
  auto& label(const vtkStdString& text)
  {
    object()->SetLabel(text);
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
    dynamic_cast<vtkPlotPoints*>(object())->SetMarkerStyle(style);
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
    return object()->GetWidth();
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
    object()->SetWidth(em);
    return *this;
  }
};

}  // namespace pdmath

#endif  // PDMATH_VTK_PLOT_H_
