/**
 * @file vtk_normal_plot.cc
 * @author Derek Huang
 * @brief C++ program using VTK to plot multiple normal PDFs and CDFs
 * @copyright MIT License
 *
 * This program plots the standard normal PDF and CDF across (-5, 5), aiming to
 * replicate the plots on https://en.wikipedia.org/wiki/Normal_distribution,
 * but only for the standard normal distribution. Program is adapted from the
 * VTK example https://examples.vtk.org/site/Cxx/Plotting/MultiplePlots/.
 */

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include <vtkAxis.h>
#include <vtkBrush.h>
#include <vtkChartXY.h>
#include <vtkContextActor.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkFloatArray.h>
#include <vtkPen.h>
#include <vtkPlot.h>
#include <vtkPlotPoints.h>
#include <vtkPNGWriter.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTable.h>
#include <vtkWindowToImageFilter.h>

#include "pdmath/normal.h"
#include "pdmath/vtk_chart.h"
#include "pdmath/vtk_table.h"

int main()
{
  // VTK named colors object + RGB double + RGBA named color helpers
  vtkNew<vtkNamedColors> colors;
  auto drgb = [&colors](auto name) { return colors->GetColor3d(name); };
  auto rgba = [&colors](auto name) { return colors->GetColor4ub(name); };
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
  auto new_chart_1 = pdmath::vtk_xy_chart{}
    .title("normal pdf")
    .color(rgba("MistyRose"))
    .opacity(0.5)
    // draw axes
    .axis(vtkAxis::BOTTOM)
      .grid_color(rgba("LightGrey"))
      .title("x")
    ()
    .axis(vtkAxis::LEFT)
      .grid_color(rgba("LightGrey"))
      .title("pdf(x)")
    ()
    // add line plots
    .plot<vtkChart::LINE>()
      .data(data, "x", "pdf(x)")
      .color(rgba("Red"))
      .width(3.)
      .label("N(0, 1)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "pdf[0, 0.2](x)")
      .color(rgba("CornflowerBlue"))
      .width(3.)
      .label("N(0, 0.2)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "pdf[0, 5](x)")
      .color(rgba("Orange"))
      .width(3.)
      .label("N(0, 5)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "pdf[-2, 0.5](x)")
      .color(rgba("Green"))
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
  auto new_chart_2 = pdmath::vtk_xy_chart{}
    .title("normal cdf")
    .color(rgba("Thistle"))
    .opacity(0.5)
    .axis(vtkAxis::BOTTOM)
      .grid_color(rgba("LightCyan"))
      .title("x")
    ()
    .axis(vtkAxis::LEFT)
      .grid_color(rgba("LightCyan"))
      .title("cdf(x)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "cdf(x)")
      .color(rgba("Red"))
      .width(3.)
      .label("N(0, 1)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "cdf[0, 0.2](x)")
      .color(rgba("CornflowerBlue"))
      .width(3.)
      .label("N(0, 0.2)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "cdf[0, 5](x)")
      .color(rgba("Orange"))
      .width(3.)
      .label("N(0, 5)")
    ()
    .plot<vtkChart::LINE>()
      .data(data, "x", "cdf[-2, 0.5](x)")
      .color(rgba("Green"))
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
  // create scenes + add plots to them
  vtkNew<vtkContextScene> scene_1;
  scene_1->AddItem(new_chart_1);
  vtkNew<vtkContextScene> scene_2;
  scene_2->AddItem(new_chart_2);
  // create actors + add scenes to them
  vtkNew<vtkContextActor> actor_1;
  actor_1->SetScene(scene_1);
  vtkNew<vtkContextActor> actor_2;
  actor_2->SetScene(scene_2);
  // create top and bottom renderers. viewport coordinates consist of
  // (x_min, y_min, x_max, y_max) as a point in 4D [0, 1] hypercube. see
  // color list used for renders via https://htmlpreview.github.io/ of
  // https://github.com/Kitware/vtk-examples/blob/gh-pages/VTKNamedColorPatches.html
  vtkNew<vtkRenderer> ren_1;
  ren_1->SetViewport(0., 0.5, 1., 1.);  // top renderer
  ren_1->SetBackground(drgb("AliceBlue").GetData());
  ren_1->SetBackgroundAlpha(0.5);       // 0 by default for transparency
  vtkNew<vtkRenderer> ren_2;
  ren_2->SetViewport(0., 0., 1., 0.5);  // bottom renderer
  ren_2->SetBackground(drgb("Lavender").GetData());
  ren_2->SetBackgroundAlpha(0.5);       // 0 by default for transparency
  // add actors to renderers
  ren_1->AddActor(actor_1);
  ren_2->AddActor(actor_2);
  // x + y dimension of each plot
  constexpr auto x_dim = 640u;
  constexpr auto y_dim = 480u;
  // create VTK rendering window that writes to framebuffer instead of screen
  vtkNew<vtkRenderWindow> win;
  win->SetWindowName("normal pdf + cdf");
  win->SetSize(x_dim, y_dim * 2);
  // TODO: #if 0 to enable window interactor
#if 1
  win->SetOffScreenRendering(1);  // hide window + use off-screen buffer
#endif  // 1
  // add renderers to window
  win->AddRenderer(ren_1);
  win->AddRenderer(ren_2);
  // create window interactor to enable rendering
  // TODO: #if 1 to start interactor event loop
#if 0
  vtkNew<vtkRenderWindowInteractor> iwin;
  iwin->SetRenderWindow(win);
#endif  // 0
  // render window and start event loop
  // note: do *not* render before setting the interactor's render window; this
  // will cause a segmentation fault when you close the window
  win->Render();
  // TODO: #if 1 to start interactor even tloop
#if 0
  iwin->Start();  // automatically calls Initialize()
#endif  // 0
  // create window filter to write to write to image
  vtkNew<vtkWindowToImageFilter> wif;
  wif->SetInput(win);
  wif->ReadFrontBufferOff();        // read back buffer only
  wif->SetInputBufferTypeToRGBA();  // RGBA input (need alpha level)
  // use PNG writer to write rendering to file
  auto png_path = std::filesystem::path{__FILE__}.replace_extension(".png");
  std::cout << "writing " << png_path.filename() << "... " << std::flush;
  vtkNew<vtkPNGWriter> pngw;
  pngw->SetFileName(png_path.c_str());
  pngw->SetInputConnection(wif->GetOutputPort());
  pngw->Write();
  std::cout << "done" << std::endl;
  return EXIT_SUCCESS;
}
