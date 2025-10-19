/**
 * @file vtk_renderer.h
 * @author Derek Huang
 * @brief C++ header for `vtkRenderer` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_RENDERER_H_
#define PDMATH_VTK_RENDERER_H_

#include <cstddef>
#include <utility>

#include <vtkColor.h>
#include <vtkNew.h>
#include <vtkProp.h>
#include <vtkRenderer.h>

#include "pdmath/type_traits.h"
#include "pdmath/vtk_skeleton.h"

namespace pdmath {

/**
 * `vtkRenderer` wrapper with fluent API.
 *
 * This provides a fluent API for the `vtkRenderer` while using the
 * `vtk_skeleton` ownership architecture to model ownership.
 *
 * @tparam P Parent type
 */
template <typename P = void>
class vtk_renderer : public vtk_skeleton<vtk_renderer<P>, vtkRenderer, P> {
public:
  PDMATH_USING_VTK_SKELETON(vtk_renderer<P>, vtkRenderer, P);

  /**
   * Add a prop to the renderer.
   *
   * For example, this could be a `vtkContextActor` with a set scene.
   *
   * @param prop Prop to add to the renderer
   */
  auto& add(vtkProp* prop)
  {
    object()->AddViewProp(prop);
    return *this;
  }

  /**
   * Add a new prop to the renderer.
   *
   * This returns the appropriate `vtk_skeleton` derived type that corresponds
   * to the provided VTK prop type using `vtk_skeleton_traits`.
   *
   * @tparam T VTK object type
   */
  template <typename T>
  auto add(constraint_t<std::is_base_of_v<vtkProp, T>> = 0)
  {
    vtkNew<T> prop;
    // note: reference increment means no use-after-free error on scope exit
    object()->AddViewProp(prop);
    return vtk_skeleton_type_t<T, vtk_renderer>{prop, this};
  }

  /**
   * Set the renderer background alpha (transparency).
   *
   * @param v Alpha value in [0, 1]
   */
  auto& alpha(double v)
  {
    object()->SetBackgroundAlpha(v);
    return *this;
  }

  /**
   * Get the renderer background alpha.
   */
  auto alpha() const
  {
    return object()->GetBackgroundAlpha();
  }

  /**
   * Set the renderer background color.
   *
   * @param rgb RGB color to use for the renderer background
   */
  auto& color(const vtkColor3d& rgb)
  {
    object()->SetBackground(rgb.GetRed(), rgb.GetGreen(), rgb.GetBlue());
    return *this;
  }

  /**
   * Set the renderer background color and transparency.
   *
   * @param rgba RGBA color to use for the renderer background.
   */
  auto& color(const vtkColor4d& rgba)
  {
    color({rgba.GetRed(), rgba.GetGreen(), rgba.GetBlue()});
    alpha(rgba.GetAlpha());
    return *this;
  }

  /**
   * Get the renderer background color.
   */
  auto color() const
  {
    return object()->GetBackground();
  }

  /**
   * Proxy type for a view of the viewport coordinates.
   *
   * This provides a type-safe way to efficiently access viewport coordinates.
   */
  class viewport_view {
  public:
    /**
     * Get the viewport's minimum x coordinate.
     */
    auto xmin() const noexcept { return data_[0]; }

    /**
     * Get the viewport's minimum y coordinate.
     */
    auto ymin() const noexcept { return data_[1]; }

    /**
     * Get the viewport's maximum x coordinate.
     */
    auto xmax() const noexcept { return data_[2]; }

    /**
     * Get the viewport's maximum y coordinate.
     */
    auto ymax() const noexcept { return data_[3]; }

  private:
    /**
     * Ctor.
     *
     * @param data Viewport coordinates (x_min, y_min, x_max, y_max)
     */
    explicit viewport_view(double* data) noexcept : data_{data} {}

    double* data_;
    friend class vtk_renderer;
  };

  /**
   * Set the renderer viewport.
   *
   * The viewport coordinates can be thought of as defining the fraction of the
   * rendering window that the rendered scene will cover.
   *
   * @param xmin Minimum x coordinate in [0, 1]
   * @param ymin Minimum y coordinate in [0, 1]
   * @param xmax Maximum x coordinate in [0, 1]
   * @param ymax Maximum y coordinate in [0, 1]
   */
  auto& viewport(double xmin, double ymin, double xmax, double ymax)
  {
    object()->SetViewport(xmin, ymin, xmax, ymax);
    return *this;
  }

  /**
   * Set the renderer viewport.
   *
   * This enables specifying the view with x and y pairs, e.g.
   *
   * @code{.cc}
   * ren.viewport({0., 1.}, {0.5, 1.});  // render on top half of window
   * @endcode
   *
   * Compared to the 4-double interface this is less confusing.
   *
   * @param xs Minimum and maximum viewport x coordinates in [0, 1] x [0, 1]
   * @param ys Minimum and maximum viewport y coordinates in [0, 1] x [0, 1]
   */
  auto& viewport(std::pair<double, double> xs, std::pair<double, double> ys)
  {
    return viewport(xs.first, ys.first, xs.second, ys.second);
  }

  /**
   * Get the renderer viewport.
   */
  auto viewport() const
  {
    return viewport_view{object()->GetViewport()};
  }
};

}  // namespace pdmath

#endif  // PDMATH_VTK_RENDERER_H_
