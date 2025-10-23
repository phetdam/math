/**
 * @file vtk_qmc_pi.cc
 * @author Derek Huang
 * @brief C++ program with VTK plots for QMC pi estimation charts
 * @copyright MIT License
 *
 * This program uses the `qmc_pi()` function to estimate pi using quasi Monte
 * Carlo via a grid-based sampling approach to show how the estimate converges.
 * Since the grid-based sampling of points is extremely cheap this program only
 * takes a couple seconds to run despite sampling millions of points.
 */

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <iostream>
#include <tuple>
#include <vector>

#include <vtkChart.h>
#include <vtkChartXY.h>
#include <vtkContextActor.h>
#include <vtkDoubleArray.h>

#include "pdmath/pi.h"
#include "pdmath/vtk_actor.h"
#include "pdmath/vtk_chart.h"
#include "pdmath/vtk_image_writer.h"
#include "pdmath/vtk_named_colors.h"
#include "pdmath/vtk_table.h"
#include "pdmath/vtk_window.h"

int main()
{
  using namespace pdmath::vtk_literals;
  // number of rows (also number of plotted data points)
  constexpr auto n_rows = 50u;
  // named colors object
  pdmath::vtk_named_colors nc;
  // lambda to convert an index into a partition count
  // note: we want max 5000 partitions for 25m points at max
  auto parts = [](auto i) { return 1000u + 80u * (i + 1u); };
  // compute async values of QMC pi estimate
  // note: this is faster than serial estimation of QMC pi
  // TODO: maybe use an actual thread pool as this is uncontrolled
  std::vector<std::future<double>> futs(n_rows);
  for (auto i = 0u; i < n_rows; i++)
    futs[i] = std::async(
      std::launch::async,
      [n = parts(i)] { return pdmath::qmc_pi(n); }
    );
  // callable to block for n (partition count), pi, and qmc_pi(n)
  auto pi_pair = [&futs, &parts](auto i /*row index*/)
  {
    // we want n to be 5000 at max for 25m points at max
    auto n = parts(i);
    return std::make_tuple(n * n, pdmath::pi, futs[i].get());
  };
  // create table with x, pi, and qmc_pi(x) data
  auto table = pdmath::vtk_table{}
    .column<vtkDoubleArray>("n * n")
    .column<vtkDoubleArray>("pi")
    .column<vtkDoubleArray>("qmc_pi(n)")
    .rows(n_rows, pi_pair)
    ();
  // add + render VTK window that renders the QMC chart to an off-screen buffer
  auto win = pdmath::vtk_window{}
    .size(640, 480)          // 4:3 ratio
    .off_screen_rendering()
    .renderer()
      .viewport({0., 1.}, {0., 1.})   // render on entire window
      .color(nc("Lavender"_3d), 0.5)  // default opacity is 0
      .add<vtkContextActor>()
        .scene()
          .add<vtkChartXY>()
            .axis(vtkAxis::BOTTOM)
              .title("n * n")
            ()
            .axis(vtkAxis::LEFT)
              .title("qmc_pi(n)")
            ()
            .title("QMC pi")
            .color(nc("Thistle"_4ub))
            .opacity(0.5)
            // pi constant plot
            .plot<vtkChart::LINE>()
              .data(table, "n * n", "pi")
              .color(nc("Red"_4ub))
              .width(3.)
              .label("pi")
            ()
            // qmc_pi(x) plot
            .plot<vtkChart::LINE>()
              .data(table, "n * n", "qmc_pi(n)")
              .color(nc("CornflowerBlue"_4ub))
              .width(3.)
              .label("qmc_pi(n)")
            ()
            .legend()
              .show()
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
