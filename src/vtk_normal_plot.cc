/**
 * @file vtk_normal_plot.cc
 * @author Derek Huang
 * @brief C++ program using VTK to plot multiple normal PDFs and CDFs
 * @copyright MIT License
 *
 * This program plots the standard normal PDF and CDF across (-5, 5), aiming to
 * replicate the plots on https://en.wikipedia.org/wiki/Normal_distribution.
 * Program is heavily modified and resdesigned from the VTK example
 * https://examples.vtk.org/site/Cxx/Plotting/MultiplePlots/.
 */

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include <vtkAxis.h>
#include <vtkBrush.h>
#include <vtkChart.h>
#include <vtkChartLegend.h>
#include <vtkContextActor.h>
#include <vtkFloatArray.h>
#include <vtkNew.h>
#include <vtkPen.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTable.h>

#include "pdmath/normal.h"
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
  // callable to fill each table row with
  auto make_row = [](auto i, auto n_rows)
  {
    // x values lie in [-5, 5] and we want them to avoid the endpoints. this
    // allows the points, if they were in (0, 1), to be the sequence
    // (0.5 / n_rows, ... (n_rows - 0.5) / n_rows)
    auto x = -5 + 10 * (0.5 + i) / n_rows;
    // precompute standard deviations
    auto root_0_2 = std::sqrt(0.2);
    auto root_5 = std::sqrt(5.);
    auto root_0_5 = std::sqrt(0.5);
    // set, pdf(x), and cdf(x) values
    return std::make_tuple(
      x,
      pdmath::normal::pdf(x),                              // N(0, 1)
      pdmath::normal::pdf(x / root_0_2) / root_0_2,        // N(0, 0.2)
      pdmath::normal::pdf(x / root_5) / root_5,            // N(0, 5)
      pdmath::normal::pdf((x + 2) / root_0_5) / root_0_5,  // N(-2, 0.5)
      pdmath::normal::cdf(x),                              // N(0, 1)
      pdmath::normal::cdf(x / root_0_2),                   // N(0, 0.2)
      pdmath::normal::cdf(x / root_5),                     // N(0, 5)
      pdmath::normal::cdf((x + 2) / root_0_5)              // N(-2, 0.5)
    );
  };
  // create table with x values, pdf(x), and cdf(x) values
  auto data = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("pdf(x)")
    .column<vtkFloatArray>("pdf[0, 0.2](x)")
    .column<vtkFloatArray>("pdf[0, 5](x)")
    .column<vtkFloatArray>("pdf[-2, 0.5](x)")
    .column<vtkFloatArray>("cdf(x)")
    .column<vtkFloatArray>("cdf[0, 0.2](x)")
    .column<vtkFloatArray>("cdf[0, 5](x)")
    .column<vtkFloatArray>("cdf[-2, 0.5](x)")
    .rows(80, make_row)
    ();
  // add top chart for normal PDF plots
  auto chart_1 = pdmath::vtk_xy_chart{}
    .title("normal pdf")
    .color(nc("MistyRose"_4ub))
    .opacity(0.5)
    // draw axes
    .axis(vtkAxis::BOTTOM)
      .grid_color(nc("LightGrey"_4ub))
      .title("x")
    ()
    .axis(vtkAxis::LEFT)
      .grid_color(nc("LightGrey"_4ub))
      .title("pdf(x)")
    ()
    // add line plots
    .plot<vtkChart::LINE>()
      .data(data, "x", "pdf(x)")
      .color(nc("Red"_4ub))
      .width(3.)
      .label("N(0, 1)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "pdf[0, 0.2](x)")
      .color(nc("CornflowerBlue"_4ub))
      .width(3.)
      .label("N(0, 0.2)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "pdf[0, 5](x)")
      .color(nc("Orange"_4ub))
      .width(3.)
      .label("N(0, 5)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "pdf[-2, 0.5](x)")
      .color(nc("Green"_4ub))
      .width(3.)
      .label("N(-2, 0.5)")
    ()
    // move legend and make visible
    .legend()
      .align<vtkChartLegend::LEFT>()
      .show()
    ()
    ();
  // add bottom chart for normal CDF plots
  auto chart_2 = pdmath::vtk_xy_chart{}
    .title("normal cdf")
    .color(nc("Thistle"_4ub))
    .opacity(0.5)
    .axis(vtkAxis::BOTTOM)
      .grid_color(nc("LightCyan"_4ub))
      .title("x")
    ()
    .axis(vtkAxis::LEFT)
      .grid_color(nc("LightCyan"_4ub))
      .title("cdf(x)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "cdf(x)")
      .color(nc("Red"_4ub))
      .width(3.)
      .label("N(0, 1)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "cdf[0, 0.2](x)")
      .color(nc("CornflowerBlue"_4ub))
      .width(3.)
      .label("N(0, 0.2)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "cdf[0, 5](x)")
      .color(nc("Orange"_4ub))
      .width(3.)
      .label("N(0, 5)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "cdf[-2, 0.5](x)")
      .color(nc("Green"_4ub))
      .width(3.)
      .label("N(-2, 0.5)")
    ()
    .legend()
      // more explicitly:
      // .align<vtkChartLegend::LEFT, pdmath::vtk_no_align>()
      .align<vtkChartLegend::LEFT>()
      .show()
    ()
    ();
  // x + y dimension of each plot
  constexpr auto x_dim = 640u;
  constexpr auto y_dim = 480u;
  // create VTK rendering window that writes to framebuffer instead of screen
  auto win = pdmath::vtk_window{}
    .name("normal pdf + cdf")
    .size(x_dim, 2 * y_dim)
    .off_screen_rendering()            // hide window + use off-screen buffer
    // top renderer with normal PDF plots. see color list used for renderer
    // backgrounds via https://htmlpreview.github.io/ of the following:
    // https://github.com/Kitware/vtk-examples/blob/gh-pages/VTKNamedColorPatches.html
    .renderer()
      .viewport({0., 1.}, {0.5, 1.})   // render on top half of window
      .color(nc("AliceBlue"_3d), 0.5)  // 0 by default for transparency
      .add<vtkContextActor>()
        .scene()
          .add(chart_1)
        ()
      ()
    ()
    // bottom renderer with normal CDF plots
    .renderer()
      .viewport({0., 1.}, {0., 0.5})   // render on top half of window
      .color(nc("Lavender"_3d), 0.5)   // 0 by default for transparency
      .add<vtkContextActor>()
        .scene()
          .add(chart_2)
        ()
      ()
    ()
    ();
  // create window interactor to enable rendering
  // TODO: #if 1 to start interactor event loop
#if 0
  vtkNew<vtkRenderWindowInteractor> iwin;
  iwin->SetRenderWindow(win);
#endif  // 0
  // render window and start event loop
  // note: if there is no display this will hard abort
  // note: do *not* render before setting the interactor's render window; this
  // will cause a segmentation fault when you close the window
  win.render();
  // TODO: #if 1 to start interactor event loop
#if 0
  iwin->Start();  // automatically calls Initialize()
#endif  // 0
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
