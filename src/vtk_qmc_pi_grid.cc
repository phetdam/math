/**
 * @file vtk_qmc_pi_grid.cc
 * @author Derek Huang
 * @brief C++ program to plot the QMC pi estimation grid
 * @copyright MIT License
 *
 * This plots the grid of points used for quasi Monte Carlo estimation of pi by
 * `qmc_pi()` and also plots the upper right quarter of the unit circle that is
 * approximated by the quasi Monte Carlo grid.
 */

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <tuple>

#include <vtkAxis.h>
#include <vtkChart.h>
#include <vtkChartXY.h>
#include <vtkContextActor.h>
#include <vtkFloatArray.h>
#include <vtkObject.h>
#include <vtkPlotPoints.h>

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
  // named colors object
  pdmath::vtk_named_colors nc;
  // number of points to draw along an axis
  constexpr auto n_axis_pts = 20u;
  // table plotting the unit circle boundary
  auto uc_data = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("y")
    .rows(
      250u,  // fine partition makes the line plot look smoother
      [](auto i, auto n_rows)
      {
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5219)
        // we want points in [0, 1] and wish to include both endpoints
        auto x = (i + 0.f) / (n_rows - 1);
PDMATH_MSVC_WARNINGS_POP()
        // back out y value by taking square root of 1 - x * x
        return std::make_tuple(x, std::sqrt(1 - x * x));
      }
    )
    ();
  // table with QMC point grid
  auto qmc_data = pdmath::vtk_table{}
    .column<vtkFloatArray>("x")
    .column<vtkFloatArray>("y")
    .rows(
      n_axis_pts * n_axis_pts,  // total point count is the square
      [](auto i)
      {
PDMATH_MSVC_WARNINGS_PUSH()
PDMATH_MSVC_WARNINGS_DISABLE(5219)
        return std::make_tuple(
          // repeat x points {0.5 / n_axis_pts, ... 1 - 0.5 / n_axis_pts}
          // n_axis_pts times, with each n_axis_pts getting the same y value
          ((i % n_axis_pts) + 0.5f) / n_axis_pts,
          // y is constant for every n_axis_pts x values
          (i / n_axis_pts + 0.5f) / n_axis_pts
PDMATH_MSVC_WARNINGS_POP()
        );
      }
    )
    ();
  // add + render VTK window
  auto win = pdmath::vtk_window{}
    .size(600u, 600u)
    .off_screen_rendering()
    .renderer()
      .viewport({0., 1.}, {0., 1.})   // render on entire window
      .color(nc("Lavender"_3d), 0.5)  // default opacity is 0
      .add<vtkContextActor>()
        .scene()
          .add<vtkChartXY>()
            // only display axis endpoints and hide grid
            .axis(vtkAxis::BOTTOM)
              .hide_grid()
              .hide_title()
              .label_offset(10)  // plot looks nicer with labels further away
              .ticks(2)
            ()
            .axis(vtkAxis::LEFT)
              .hide_grid()
              .hide_title()
              .label_offset(10)  // plot looks nicer with labels further away
              .ticks(2)
            ()
            .color(nc("Thistle"_4ub))
            .opacity(0.5)
            // unit circle boundary plot
            .plot<vtkChart::LINE>()
              .data(uc_data, "x", "y")
              .color(nc("Red"_4ub))
            ()
            // plot QMC grid points
            .plot<vtkChart::POINTS>()
              .data(qmc_data, "x", "y")
              .color(nc("Blue"_4ub))
              .marker(vtkPlotPoints::PLUS)
            ()
          ()
        ()
      ()
    ()
    .render()
    ();
  // output path for PNG
  auto png_path = std::filesystem::path{__FILE__}.replace_extension(".png");
  // create PNG pipeline for window that writes to file
  std::cout << "writing " << png_path.filename() << "... " << std::flush;
  pdmath::vtk_png_pipeline{}
    .input(win)
    .rgba()
    .output(png_path)
    ();
  std::cout << "done" << std::endl;
  return EXIT_SUCCESS;
}
