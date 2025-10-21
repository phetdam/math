/**
 * @file vtk_trig_plot.cc
 * @author Derek Huang
 * @brief C++ program using VTK to plot sin, cos, and tan functions
 * @copyright MIT License
 */

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <tuple>

#include <vtkAxis.h>
#include <vtkChart.h>
#include <vtkChartLegend.h>
#include <vtkContextActor.h>
#include <vtkFloatArray.h>

#include "pdmath/vtk_actor.h"
#include "pdmath/vtk_chart.h"
#include "pdmath/vtk_image_writer.h"
#include "pdmath/vtk_named_colors.h"
#include "pdmath/vtk_renderer.h"
#include "pdmath/vtk_table.h"
#include "pdmath/vtk_window.h"

int main()
{
  using namespace pdmath::vtk_literals;
  // VTK named colors object
  pdmath::vtk_named_colors nc;
  // callable to fill table rows with
  auto make_row = [](auto i, auto n_rows)
  {
    // points in (-1.2, 1.2). we avoid the left endpoint by using the sequence
    // of points {0.5 / n_rows, ... (n_rows - 0.5) / n_rows} before we do the
    // affine transform to ensure points are in the desired interval
    auto x = -1.2 + 2.4 * (0.5 + i) / n_rows;
    // row is composed of x, sin(x), cos(x), tan(x)
    return std::make_tuple(x, std::sin(x), std::cos(x), std::tan(x));
  };
  // number of points to generate (also number of table rows)
  constexpr auto n_points = 50u;
  // create + fill table with float columns
  auto table = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("sin(x)")
    .column<vtkFloatArray>("cos(x)")
    .column<vtkFloatArray>("tan(x)")
    .rows(n_points, make_row)
    ();
  // create the chart with sin, cos, tan plots
  auto chart = pdmath::vtk_xy_chart{}
    // set up title, background color, and background color opacity
    .title("sin, cos, tan")
    .color(nc("Thistle"_4ub))
    .opacity(0.5)
    // update x (bottom) and y (left) axis grid line colors + titles
    .axis(vtkAxis::BOTTOM)
      .grid_color(nc("LightCyan"_4ub))
      .title("x")
    ()
    .axis(vtkAxis::LEFT)
      .grid_color(nc("LightCyan"_4ub))
      .title("y")
    ()
    // add sin(x), cos(x), and tan(x) line plots
    .plot<vtkChart::LINE>()
      .data(table, "x", "sin(x)")
      .color(nc("Red"_4ub))
      .width(3.)
      .label("sin(x)")
    ()
    .plot<vtkChart::LINE>()
      .data(table, "x", "cos(x)")
      .color(nc("CornflowerBlue"_4ub))
      .width(3.)
      .label("cos(x)")
    ()
    .plot<vtkChart::LINE>()
      .data(table, "x", "tan(x)")
      .color(nc("Green"_4ub))
      .width(3.)
      .label("tan(x)")
    ()
    // add visible legend aligned to top left so it doesn't cover tan(x)
    .legend()
      .align<vtkChartLegend::LEFT, vtkChartLegend::TOP>()
      .show()
    ()
    ();
  // window dimensions
  constexpr auto x_dim = 640u;
  constexpr auto y_dim = 480u;
  // add VTK window writing to framebuffer instead of screen with renderer
  auto win = pdmath::vtk_window{}
    .name("sin, cos, tan")            // not shown if rendering off-screen
    .size(x_dim, y_dim)
    .off_screen_rendering()
    // renderer with sin, cos, tan line plot chart
    .renderer()
      .viewport({0., 1.}, {0., 1.})   // {xmin, xmax}, {ymin, ymax}
      .color(nc("Lavender"_3d), 0.5)  // 0 by default for transparency
      .add<vtkContextActor>()
        .scene()
          .add(chart)
        ()
      ()
    ()
    // render window
    // note: if there is no display this will hard abort
    .render()
    ();
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
