/**
 * @file xopen.cc
 * @author Derek Huang
 * @brief C++ program that attempts to connect to the default X display
 * @copyright MIT License
 */

#include <cstdlib>
#include <iostream>

#include <X11/Xlib.h>

int main()
{
  // try to connect to the default display
  auto dsp = XOpenDisplay(nullptr);
  // failed if nullptr
  if (!dsp) {
    // default display connection taken from DISPLAY
    auto dsp_name = []
    {
      auto name = std::getenv("DISPLAY");
      return (name) ? name : "(unknown)";
    }();
    std::cerr << "Error: Unable to connect to display " << dsp_name << std::endl;
    return EXIT_FAILURE;
  }
  // print some basic info
  std::cout <<
    "X display: " <<
      DisplayString(dsp) << "\n" <<
    "X vendor: " <<
      ServerVendor(dsp) << "\n" <<
    "X version: " <<
      ProtocolVersion(dsp) << "." << ProtocolRevision(dsp) << "\n" << std::flush;
  // clean up
  XCloseDisplay(dsp);
  return EXIT_SUCCESS;
}
