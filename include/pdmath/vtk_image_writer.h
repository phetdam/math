/**
 * @file vtk_image_writer.h
 * @author Derek Huang
 * @brief C++ header for `vtkImageWriter` helpers
 * @copyright MIT License
 */

#ifndef PDMATH_VTK_IMAGE_WRITER_H_
#define PDMATH_VTK_IMAGE_WRITER_H_

#include <filesystem>

#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkWindow.h>
#include <vtkWindowToImageFilter.h>

namespace pdmath {

/**
 * PNG writer pipeline for a VTK window.
 *
 * This provides a simple way to construct the `vtkWindowToImageFilter` and
 * `vtkPNGWriter` with less verbosity. For example:
 *
 * @code{.cc}
 * // win is an existing vtkRenderWindow or vtk_window
 * vtk_png_pipeline{}
 *   .input(win)
 *   .rgba()
 *   .output("image.png")
 *   ();
 * @endcode
 */
class vtk_png_pipeline {
public:
  /**
   * Ctor.
   *
   * This connects the image filter output port to the PNG writer.
   */
  vtk_png_pipeline()
  {
    writer_->SetInputConnection(filter_->GetOutputPort());
  }

  /**
   * Set the input window.
   *
   * @param win Input window to read from
   */
  auto& input(vtkWindow* win)
  {
    filter_->SetInput(win);
    return *this;
  }

  /**
   * Get the input window.
   */
  auto input() const
  {
    return filter_->GetInput();
  }

  /**
   * Set the output file.
   *
   * @param path Output file path
   */
  auto& output(const std::filesystem::path& path)
  {
// MSVC uses wchar_t so need temporary string materialization
// note: can't use if constexpr since discarded statements are fully
// checked if used outside the definition of a function template
#if defined(_MSC_VER)
    writer_->SetFileName(path.string().c_str());
// otherwise just use c_str() directlry
#else
    writer_->SetFileName(path.c_str());
#endif  // !defined(_MSC_VER)
    return *this;
  }

  /**
   * Get the output path.
   */
  auto output() const
  {
    return std::filesystem::path{writer_->GetFileName()};
  }

  /**
   * Set whether to read fron the front buffer or not.
   *
   * @note The front buffer is the one displayed on screen while the back
   *  buffer is where scenes are drawn before copying to the front buffer.
   *
   * @param v `true` to read front buffer, `false` to read back buffer
   */
  auto& read_front_buffer(bool v)
  {
    filter_->SetReadFrontBuffer(v);
    return *this;
  }

  /**
   * Indicate if reading from front buffer.
   */
  bool read_front_buffer() const
  {
    return !!filter_->GetReadFrontBuffer();
  }

  /**
   * Set the input buffer format to RGB.
   *
   * This typically does not need to be called as it is the default.
   */
  auto& rgb()
  {
    filter_->SetInputBufferTypeToRGB();
    return *this;
  }

  /**
   * Set the input buffer format to RGBA.
   */
  auto& rgba()
  {
    filter_->SetInputBufferTypeToRGBA();
    return *this;
  }

  /**
   * Write the window output to the specified output path.
   */
  void operator()() const
  {
    writer_->Write();
  }

private:
  vtkNew<vtkPNGWriter> writer_;
  vtkNew<vtkWindowToImageFilter> filter_;
};

}  // namespace pdmath

#endif  // PDMATH_VTK_IMAGE_WRITER_H_
