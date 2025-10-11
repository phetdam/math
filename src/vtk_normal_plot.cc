/**
 * @file vtk_normal_plot.cc
 * @author Derek Huang
 * @brief C++ program using VTK to plot normal PDF and CDF
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

// TODO: add a helper to reduce boilerplate in creating a single chart

namespace {

// current program absolute path + name
const std::filesystem::path progpath{__FILE__};
const auto progname = progpath.stem().string();

}  // namespace

int main()
{
  // x + y dimension of each plot
  constexpr auto x_dim = 640u;
  constexpr auto y_dim = 480u;
  // create VTK rendering window + interactor
  vtkNew<vtkRenderWindow> win;
  vtkNew<vtkRenderWindowInteractor> iwin;
  win->SetWindowName("normal pdf + cdf");
  win->SetSize(x_dim, y_dim * 2);
  iwin->SetRenderWindow(win);
  // VTK named colors object
  vtkNew<vtkNamedColors> colors;
  // number of plots on the x and y axes
  // constexpr auto x_plots = 1u;
  // constexpr auto y_plots = 2u;
  // create top and bottom renderer viewport coordinates. each consists of
  // (x_min, y_min, x_max, y_max) as a point in 4D [0, 1] hypercube
  vtkNew<vtkRenderer> ren_1;
  ren_1->SetViewport(0., 0.5, 1., 1.);  // top renderer
  vtkNew<vtkRenderer> ren_2;
  ren_2->SetViewport(0., 0., 1., 0.5);  // bottom renderer
  // set renderer colors. see https://htmlpreview.github.io/ of
  // https://github.com/Kitware/vtk-examples/blob/gh-pages/VTKNamedColorPatches.html
  ren_1->SetBackground(colors->GetColor3d("AliceBlue").GetData());
  ren_2->SetBackground(colors->GetColor3d("Lavender").GetData());
  // add renderers to window
  win->AddRenderer(ren_1);
  win->AddRenderer(ren_2);
  // create top chart for ren_1
  vtkNew<vtkChartXY> chart_1;
  vtkNew<vtkContextScene> scene_1;
  vtkNew<vtkContextActor> actor_1;
  scene_1->AddItem(chart_1);
  actor_1->SetScene(scene_1);
  // update renderer with actor and scene with renderer
  ren_1->AddActor(actor_1);
  scene_1->SetRenderer(ren_1);
  // paint chart_1 left and bottom axes + set their titles
  auto x_axis_1 = chart_1->GetAxis(vtkAxis::BOTTOM);
  auto y_axis_1 = chart_1->GetAxis(vtkAxis::LEFT);
  // note: using 4-component RGBA so we have transparency (alpha) level
  x_axis_1->GetGridPen()->SetColor(colors->GetColor4ub("LightGrey"));
  x_axis_1->SetTitle("x");
  y_axis_1->GetGridPen()->SetColor(x_axis_1->GetGridPen()->GetColorObject());
  y_axis_1->SetTitle("pdf(x)");
  // set background + title for chart_1
  chart_1->GetBackgroundBrush()->SetColor(colors->GetColor4ub("MistyRose"));
  chart_1->GetBackgroundBrush()->SetOpacityF(0.5);
  chart_1->SetTitle("normal pdf");
  // create bottom chart for ren_2
  vtkNew<vtkChartXY> chart_2;
  vtkNew<vtkContextScene> scene_2;
  vtkNew<vtkContextActor> actor_2;
  scene_2->AddItem(chart_2);
  actor_2->SetScene(scene_2);
  // update renderer with actor and scene with renderer
  ren_2->AddActor(actor_2);
  scene_2->SetRenderer(ren_2);
  // paint chart_2 left and bottom axes + set their titles
  auto x_axis_2 = chart_2->GetAxis(vtkAxis::BOTTOM);
  auto y_axis_2 = chart_2->GetAxis(vtkAxis::LEFT);
  x_axis_2->GetGridPen()->SetColor(colors->GetColor4ub("LightCyan"));
  x_axis_2->SetTitle("x");
  y_axis_2->GetGridPen()->SetColor(x_axis_2->GetGridPen()->GetColorObject());
  y_axis_2->SetTitle("cdf(x)");
  // set background + title for chart_2
  chart_2->GetBackgroundBrush()->SetColor(colors->GetColor4ub("Thistle"));
  chart_2->GetBackgroundBrush()->SetOpacityF(0.5);
  chart_2->SetTitle("normal cdf");
  // create table with x values, pdf(x), and cdf(x) values
  vtkNew<vtkTable> data;
  vtkNew<vtkFloatArray> x_data;
  vtkNew<vtkFloatArray> pdf_data;
  vtkNew<vtkFloatArray> cdf_data;
  x_data->SetName("x");
  pdf_data->SetName("pdf(x)");
  cdf_data->SetName("cdf(x)");
  data->AddColumn(x_data);
  data->AddColumn(pdf_data);
  data->AddColumn(cdf_data);
  // set number of table rows
  constexpr auto n_points = 80u;
  data->SetNumberOfRows(n_points);
  // fill values into table. we want points in (-5, 5)
  for (auto i = 0u; i < n_points; i++) {
    // x values lie in [-5, 5] and we want them to avoid the endpoints. this
    // allows the points, if they were in (0, 1), to be
    // (0.5 / n_points, ... (n_points - 1) / n_points)
    auto x = -5 + 10 * (0.5 + i) / n_points;
    // set x, pdf(x), cdf(x)
    data->SetValue(i, 0, x);
    data->SetValue(i, 1, pdmath::normal::pdf(x));
    data->SetValue(i, 2, pdmath::normal::cdf(x));
  }
  // plotting color
  auto plot_color = colors->GetColor4ub("Violet");
  // add (x, pdf(x)) point plot to top chart (squares)
  auto plot_1 = chart_1->AddPlot(vtkChart::POINTS);
  plot_1->SetInputData(data, 0, 1);
  plot_1->SetColor(
    plot_color.GetRed(),
    plot_color.GetGreen(),
    plot_color.GetBlue(),
    plot_color.GetAlpha()
  );
  plot_1->SetWidth(1.);  // cover entire chart
  dynamic_cast<vtkPlotPoints*>(plot_1)->SetMarkerStyle(vtkPlotPoints::SQUARE);
  // add (x, cdf(x)) point plot to bottom chart (diamonds)
  auto plot_2 = chart_2->AddPlot(vtkChart::POINTS);
  plot_2->SetInputData(data, 0, 2);
  plot_2->SetColor(
    plot_color.GetRed(),
    plot_color.GetGreen(),
    plot_color.GetBlue(),
    plot_color.GetAlpha()
  );
  plot_2->SetWidth(1.);  // cover entire chart
  dynamic_cast<vtkPlotPoints*>(plot_2)->SetMarkerStyle(vtkPlotPoints::DIAMOND);
  // begin rendering window + interactor
  win->Render();
  // TODO: uncomment to start interactor event loop
  // iwin->Initialize();
  // iwin->Start();
  // create window filter to write to write to image
  vtkNew<vtkWindowToImageFilter> wif;
  wif->SetInput(win);
  wif->ReadFrontBufferOff();        // read back buffer only
  wif->SetInputBufferTypeToRGBA();  // RGBA input (need alpha level)
  // use PNG writer to write rendering to file
  vtkNew<vtkPNGWriter> pngw;
  auto png_path = progpath;
  png_path.replace_extension(".png");
  std::cout << "Writing render to " << png_path.filename() << "... " << std::flush;
  pngw->SetFileName(png_path.c_str());
  pngw->SetInputConnection(wif->GetOutputPort());
  pngw->Write();
  std::cout << "done" << std::endl;
  return EXIT_SUCCESS;
}
