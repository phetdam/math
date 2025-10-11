cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# fpi: print values of pi for several different floating types
add_executable(fpi fpi.cc)
target_link_libraries(fpi PRIVATE pdmath)
# if using GCC then quadmath is also required
if(quadmath_FOUND)
    target_link_libraries(fpi PRIVATE quadmath)
endif()
# simple smoke test
add_test(NAME fpi COMMAND fpi)

# vtk_normal_plot: plot standard normal PDF and CDF with VTK
if(VTK_FOUND)
    add_executable(vtk_normal_plot vtk_normal_plot.cc)
    # note: too many modules to keep track of so just use VTK_LIBRARIES
    target_link_libraries(vtk_normal_plot PRIVATE ${VTK_LIBRARIES})
    vtk_module_autoinit(TARGETS vtk_normal_plot MODULES ${VTK_LIBRARIES})
else()
    message(STATUS "Skipping vtk_normal_plot (requires VTK)")
endif()
