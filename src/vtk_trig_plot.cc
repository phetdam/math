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

#include <vtkContextActor.h>
#include <vtkContextScene.h>
#include <vtkFloatArray.h>
#include <vtkPNGWriter.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkWindowToImageFilter.h>

#include "pdmath/vtk_chart.h"
#include "pdmath/vtk_named_colors.h"
#include "pdmath/vtk_table.h"

// TODO: figure out how to simplify window + renderer stuff

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
  // add scene with chart
  vtkNew<vtkContextScene> scene;
  scene->AddItem(chart);
  // add actor with scene
  vtkNew<vtkContextActor> actor;
  actor->SetScene(scene);
  // add renderer with actor
  vtkNew<vtkRenderer> ren;
  ren->SetViewport(0., 0., 1., 1.);  // x_min, y_min, x_max, y_max
  ren->SetBackground(nc("Lavender"_3d).GetData());
  // note: must explicitly set alpha (0 by default for transparency)
  ren->SetBackgroundAlpha(0.5);
  ren->AddActor(actor);
  // rendering dimensions
  constexpr auto x_dim = 640u;
  constexpr auto y_dim = 480u;
  // add VTK window writing to framebuffer instead of screen with renderer
  vtkNew<vtkRenderWindow> win;
  win->SetWindowName("sin, cos, tan");  // not shown if rendering off-screen
  win->SetSize(x_dim, y_dim);
  win->OffScreenRenderingOn();
  win->AddRenderer(ren);
  // begin rendering window
  // note: if there is no display this will hard abort
  win->Render();
  // create window filter to write to write to image
  vtkNew<vtkWindowToImageFilter> wif;
  wif->SetInput(win);
  wif->ReadFrontBufferOff();        // read back buffer only
  wif->SetInputBufferTypeToRGBA();  // RGBA input (need alpha level)
  // use PNG writer to write rendering to file
  vtkNew<vtkPNGWriter> pngw;
  auto png_path = std::filesystem::path{__FILE__}.replace_extension(".png");
  std::cout << "writing " << png_path.filename() << "... " << std::flush;
  pngw->SetFileName(png_path.c_str());
  pngw->SetInputConnection(wif->GetOutputPort());
  pngw->Write();
  std::cout << "done" << std::endl;
  return EXIT_SUCCESS;
}
