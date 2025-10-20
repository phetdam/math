/**
 * @file vtk_window.h
 * @author Derek Huang
 * @brief C++ header for `vtkRenderWindow` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_WINDOW_H_
#define PDMATH_VTK_WINDOW_H_

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include "pdmath/vtk_skeleton.h"

namespace pdmath {

/**
 * `vtkRenderWindow` wrapper with fluent API.
 *
 * @todo Add `renderers()` member function that returns a proxy range over the
 *  `vtkRendererCollection*` returned by `GetRenderers()`.
 *
 * @tparam T Parent type
 */
template <typename T = void>
class vtk_window : public vtk_skeleton<vtk_window<T>, vtkRenderWindow, T> {
public:
  PDMATH_USING_VTK_SKELETON(vtk_window<T>, vtkRenderWindow, T);

  /**
   * Add an existing renderer to the window.
   *
   * @param ren Renderer to add to list of window renderers
   */
  auto& add(vtkRenderer* ren)
  {
    object()->AddRenderer(ren);
    return *this;
  }

  /**
   * Add a new renderer to the window.
   *
   * This returns the `vtk_skeleton` derived type for `vtkRenderer`.
   */
  auto renderer()
  {
    vtkNew<vtkRenderer> ren;
    add(ren);
    return vtk_skeleton_type_t<vtkRenderer, vtk_window>{ren, this};
  }

  /**
   * Return the name of the rendering backend used by the window.
   */
  auto backend() const
  {
    return object()->GetRenderingBackend();
  }

  /**
   * Indicate if the specified renderer is in the list of renderers
   *
   * @param ren Renderer to check for membership
   */
  bool contains(vtkRenderer* ren) const
  {
    return !!object()->HasRenderer(ren);
  }

  /**
   * Return the detected screen DPI.
   */
  auto dpi() const
  {
    return object()->GetDPI();
  }

  /**
   * Set the screen DPI.
   *
   * @param v DPI value between 1 and `INT_MAX`
   */
  auto dpi(unsigned v)
  {
    object()->SetDPI(static_cast<int>(v));
    return *this;
  }

  /**
   * Set whether or not the window should render in full screen mode.
   *
   * @param v `true` to render in full screen, `false` not to
   */
  auto& fullscreen(bool v = true)
  {
    object()->SetFullScreen(v);
    return *this;
  }

  /**
   * Indicate if the window will render in full screen mode or now.
   */
  bool is_fullscreen() const
  {
    return object()->GetFullScreen();
  }

  /**
   * Return the name of the rendering library used by the window.
   */
  auto library() const
  {
    return object()->GetRenderLibrary();
  }

  /**
   * Try to hide the window and use off screen buffers to render.
   *
   * This determines whether to show and window and whether to use off screen
   * buffers in a single call vs. calling `show(false)` and
   * `off_screen_buffers(false)` as separate calls.
   *
   * @param v `true` to
   */
  auto& off_screen_rendering(bool v = true)
  {
    object()->SetOffScreenRendering(v);
    return *this;
  }

  /**
   * Get the window name.
   */
  auto name() const
  {
    return object()->GetWindowName();
  }

  /**
   * Set the window name.
   *
   * @param text Text to use as the window name
   */
  auto& name(const char* text)
  {
    object()->SetWindowName(text);
    return *this;
  }

  /**
   * Ask each renderer in the window's renderer list to render its image.
   *
   * This also synchronizes the process according to VTK documentation.
   */
  auto& render()
  {
    object()->Render();
    return *this;
  }

  /**
   * Set whether or not the window should be shown.
   *
   * @param v `true` to show the window, `false` to hide the window
   */
  auto& show(bool v = true)
  {
    object()->SetShowWindow(v);
    return *this;
  }

  /**
   * Indicate if the window is showing or not.
   */
  bool showing() const
  {
    return object()->GetShowWindow();
  }

  /**
   * Proxy type for a view of the window width and height.
   */
  class window_size {
  public:
    /**
     * Return the window width.
     */
    auto width() const noexcept { return data_[0]; }

    /**
     * Return the window height.
     */
    auto height() const noexcept { return data_[1]; }

  private:
    /**
     * Ctor.
     *
     * @param data Window/screen width and height
     */
    window_size(int* data) noexcept : data_{data} {}

    int* data_;
    friend class vtk_window;
  };

  /**
   * Get the window size.
   */
  auto size() const
  {
    return window_size{object()->GetSize()};
  }

  /**
   * Set the window width and height in pixels.
   *
   * @param w Window width in pixels
   * @param h Window height in pixels
   */
  auto& size(unsigned w, unsigned h)
  {
    // note: most screens won't have (1 << CHAR_BIT * (sizeof(int) - 1)) pixels
    object()->SetSize(static_cast<int>(w), static_cast<int>(h));
    return *this;
  }
};

/**
 * Partial specialization for `vtk_skeleton_type`.
 *
 * @tparam P Parent type
 */
template <typename P>
struct vtk_skeleton_type<vtkRenderWindow, P> {
  using type = vtk_window<P>;
};

}  // namespace pdmath

#endif  // PDMATH_VTK_WINDOW_H_
