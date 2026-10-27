/**
 * @file vtk_akima_plot.cc
 * @author Derek Huang
 * @brief C++ program using VTK to plot Akima splines for assorted knot data
 * @copyright MIT License
 *
 * This program plots knot points and dotted lines for the fitted Akima spline
 * using the same knot points and data as on the MATLAB `makima` page
 * https://www.mathworks.com/help/matlab/ref/makima.html.
 *
 * In total, 4 plots are produced, with positions denoted (row, col), where
 * the origin is placed at the top left corner of the rendering area:
 *
 * 1. (0, 0) x = {0, 1, 2.5, 3.6, 5, 7, 8.1, 10}, y = cos(x)
 * 2. (1, 0) x = {-3, -2, -1, 0, 1, 2, 3}, y = {-1, -1, -1, 0, 1, 1, 1}
 * 3. (0, 1) x = {0, 1, ... 14, 15}, y = cyl_bessel_j(1, x)
 * 4. (1, 1) x = {-5, -4, ... 4, 5}, y = {1, 1, 1, 0, 0, 1, 1, 2, 2, 2, 2}
 */

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <tuple>

#include <vtkAxis.h>
#include <vtkChart.h>
#include <vtkChartLegend.h>
#include <vtkContextActor.h>
#include <vtkFloatArray.h>
#include <vtkPen.h>

#include "pdmath/akima_spline.h"
#include "pdmath/vtk_actor.h"
#include "pdmath/vtk_chart.h"
#include "pdmath/vtk_image_writer.h"
#include "pdmath/vtk_named_colors.h"
#include "pdmath/vtk_table.h"
#include "pdmath/vtk_window.h"
#include "pdmath/warnings.h"

int main()
{
  using namespace pdmath::vtk_literals;
// on Windows, disable logging of warnings
#ifdef _WIN32
  vtkObject::GlobalWarningDisplayOff();
#endif  // _WIN32
  // number of interpolation sample evaluations
  constexpr auto n_ev = 100u;
  // VTK named colors object
  pdmath::vtk_named_colors nc;
  // cosine data + spline
  std::array x1{0.f, 1.f, 2.5f, 3.6f, 5.f, 7.f, 8.1f, 10.f};
  auto y1 = x1;
  std::for_each(y1.begin(), y1.end(), [](auto& x) { x = std::cos(x); });
  pdmath::akima_spline f1{x1, y1};
  // top-left chart's cosine data
  auto data_1 = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("cos(x)")
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4365)
    .rows(x1.size(), [&](auto i) { return std::tuple{x1[i], y1[i]}; })
PDMATH_MSVC_WARNINGS_POP()
    ();
  // top-left chart's Akima evaluations
  auto akima_1 = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("f1(x)")
    .rows(
      n_ev,
      [&](auto i)
      {
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5219)
        auto x = 10.f * i / n_ev;
PDMATH_MSVC_WARNINGS_POP()
        return std::tuple{x, f1(x)};
      }
    )
    ();
  // top-left chart for cosine data + Akima evaluations
  auto chart_1 = pdmath::vtk_xy_chart{}
    .title("cosine")
    .axis(vtkAxis::BOTTOM)
      .grid_color(nc("LightGrey"_4ub))
      .title("x")
    ()
    .axis(vtkAxis::LEFT)
      .grid_color(nc("LightGrey"_4ub))
      .title("f(x)")
    ()
    // note: plot points *after* evaluations so they are on top
    .plot<vtkChart::LINE>()
      .data(akima_1, "x", "f1(x)")
      .color(nc("Orange"_4ub))
      .line(vtkPen::DASH_LINE)
      .label("akima(x)")
    ()
    .plot<vtkChart::POINTS>()
      .data(data_1, "x", "cos(x)")
      .color(nc("CornflowerBlue"_4ub))
      .label("cos(x)")
    ()
    .legend()
      .align<vtkChartLegend::RIGHT>()
      .show()
    ()
    ();
  // piecewise linear function data + spline
  std::array x2{-3.f, -2.f, -1.f, 0.f, 1.f, 2.f, 3.f};
  std::array y2{-1.f, -1.f, -1.f, 0.f, 1.f, 1.f, 1.f};
  pdmath::akima_spline f2{x2, y2};
  // bottom-left chart's piecewise linear function data
  auto data_2 = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("y")
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4365)
    .rows(x2.size(), [&](auto i) { return std::tuple{x2[i], y2[i]}; })
PDMATH_MSVC_WARNINGS_POP()
    ();
  // bottom-left chart's Akima evaluations
  auto akima_2 = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("f2(x)")
    .rows(
      n_ev,
      [&](auto i)
      {
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5219)
        auto x = 6.f * i / n_ev - 3;
PDMATH_MSVC_WARNINGS_POP()
        return std::tuple{x, f2(x)};
      }
    )
    ();
  // bottom-left chart for piecewise linear data + Akima evaluations
  auto chart_2 = pdmath::vtk_xy_chart()
    .title("piecewise linear")
    .axis(vtkAxis::BOTTOM)
      .grid_color(nc("LightGrey"_4ub))
      .title("x")
    ()
    .axis(vtkAxis::LEFT)
      .grid_color(nc("LightGrey"_4ub))
      .title("f(x)")
    ()
    .plot<vtkChart::LINE>()
      .data(akima_2, "x", "f2(x)")
      .color(nc("Orange"_4ub))
      .line(vtkPen::DASH_LINE)
      .label("akima(x)")
    ()
    .plot<vtkChart::POINTS>()
      .data(data_2, "x", "y")
      .color(nc("CornflowerBlue"_4ub))
      .label("pl(x)")
    ()
    .legend()
      .align<vtkChartLegend::LEFT>()
      .show()
    ()
    ();
  // Bessel function data + spline
  std::array<float, 16u> x3{};
  std::iota(x3.begin(), x3.end(), 0.f);
  auto y3 = x3;
  std::for_each(y3.begin(), y3.end(), [](auto& x) { x = std::cyl_bessel_j(1, x); });
  pdmath::akima_spline f3{x3, y3};
  // top-right chart's Bessel function data
  auto data_3 = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("J(1, x)")
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4365)
    .rows(x3.size(), [&](auto i) { return std::tuple{x3[i], y3[i]}; })
PDMATH_MSVC_WARNINGS_POP()
    ();
  // top-right chart's Akima evaluations
  auto akima_3 = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("f3(x)")
    .rows(
      n_ev,
      [&](auto i)
      {
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5219)
        auto x = 15.f * i / n_ev;
PDMATH_MSVC_WARNINGS_POP()
        return std::tuple{x, f3(x)};
      }
    )
    ();
  // top-right chart for Bessel function data + Akima evaluations
  auto chart_3 = pdmath::vtk_xy_chart()
    .title("bessel")
    .axis(vtkAxis::BOTTOM)
      .grid_color(nc("LightGrey"_4ub))
      .title("x")
    ()
    .axis(vtkAxis::LEFT)
      .grid_color(nc("LightGrey"_4ub))
      .title("f(x)")
    ()
    .plot<vtkChart::LINE>()
      .data(akima_3, "x", "f3(x)")
      .color(nc("Orange"_4ub))
      .line(vtkPen::DASH_LINE)
      .label("akima(x)")
    ()
    .plot<vtkChart::POINTS>()
      .data(data_3, "x", "J(1, x)")
      .color(nc("CornflowerBlue"_4ub))
      .label("J(1, x)")
    ()
    .legend()
      .align<vtkChartLegend::RIGHT>()
      .show()
    ()
    ();
  // piecewise polynomial data + spline
  std::array x4{-5.f, -4.f, -3.f, -2.f, -1.f, 0.f, 1.f, 2.f, 3.f, 4.f, 5.f};
  std::array y4{1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 2.f, 2.f, 2.f, 2.f};
  pdmath::akima_spline f4{x4, y4};
  // bottom-right chart's piecewise polynomial data
  auto data_4 = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("y")
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(4365)
    .rows(x4.size(), [&](auto i) { return std::tuple{x4[i], y4[i]}; })
PDMATH_MSVC_WARNINGS_POP()
    ();
  // bottom-right chart's Akima evaluations
  auto akima_4 = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("f4(x)")
    .rows(
      n_ev,
      [&](auto i)
      {
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5219)
        auto x = 10.f * i / n_ev - 5;
PDMATH_MSVC_WARNINGS_POP()
        return std::tuple{x, f4(x)};
      }
    )
    ();
  // bottom-right chart for piecewise polynomal data + Akima evaluations
  // TODO: increase padding so points are cut off on top right
  auto chart_4 = pdmath::vtk_xy_chart()
    .title("piecewise polynomial")
    .axis(vtkAxis::BOTTOM)
      .grid_color(nc("LightGrey"_4ub))
      .title("x")
    ()
    .axis(vtkAxis::LEFT)
      .grid_color(nc("LightGrey"_4ub))
      .title("f(x)")
    ()
    .plot<vtkChart::LINE>()
      .data(akima_4, "x", "f4(x)")
      .color(nc("Orange"_4ub))
      .line(vtkPen::DASH_LINE)
      .label("akima(x)")
    ()
    .plot<vtkChart::POINTS>()
      .data(data_4, "x", "y")
      .color(nc("CornflowerBlue"_4ub))
      .label("pp(x)")
    ()
    .legend()
      .align<vtkChartLegend::LEFT>()
      .show()
    ()
    ();
  // x + y dimension of each plot
  constexpr auto x_dim = 640u;
  constexpr auto y_dim = 480u;
  // create VTK rendering window that writes to framebuffer instead of scren
  auto win = pdmath::vtk_window{}
    .name("Akima splines")
    .size(2 * x_dim, 2 * y_dim)
    .off_screen_rendering()
    // top-left renderer
    .renderer()
      .viewport({0., 0.5}, {0.5, 1.})  // render on top-left quadrant
      .color(nc("AliceBlue"_3d), 0.5)  // 0 by default for transparency
      .add<vtkContextActor>()
        .scene()
          .add(chart_1)
        ()
      ()
    ()
    // bottom-left renderer
    .renderer()
      .viewport({0., 0.5}, {0., 0.5})  // render on bottom-left quadrant
      .color(nc("Lavender"_3d), 0.5)   // 0 by default for transparency
      .add<vtkContextActor>()
        .scene()
          .add(chart_2)
        ()
      ()
    ()
    // top-right renderer
    .renderer()
      .viewport({0.5, 1.}, {0.5, 1.})  // render on top-right quadrant
      .color(nc("Lavender"_3d), 0.5)   // 0 by default for transparency
      .add<vtkContextActor>()
        .scene()
          .add(chart_3)
        ()
      ()
    ()
    // bottom-right renderer
    .renderer()
      .viewport({0.5, 1.}, {0., 0.5})  // render on bottom-right quadrant
      .color(nc("AliceBlue"_3d), 0.5)  // 0 by default for transparency
      .add<vtkContextActor>()
        .scene()
          .add(chart_4)
        ()
      ()
    ()
    ();
  // render window and start event loop
  win.render();
  // output path for PNG
  auto png_path = std::filesystem::path{__FILE__}.replace_extension(".png");
  // create new PNG pipeline for window and write to file
  std::cout << "writing " << png_path.filename() << "... " << std::flush;
  pdmath::vtk_png_pipeline{}
    .input(win)
    .rgba()
    .output(png_path)
    ();
  std::cout << "done" << std::endl;
  return EXIT_SUCCESS;
}
