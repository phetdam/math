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

# vtk_normal_plot: plot multiple normal PDFs and CDFs with VTK
if(VTK_FOUND)
    add_executable(vtk_normal_plot vtk_normal_plot.cc)
    # note: too many modules to keep track of so just use VTK_LIBRARIES
    target_link_libraries(vtk_normal_plot PRIVATE ${VTK_LIBRARIES})
    vtk_module_autoinit(TARGETS vtk_normal_plot MODULES ${VTK_LIBRARIES})
else()
    message(STATUS "Skipping vtk_normal_plot (requires VTK)")
endif()

# fib_50: print the first 50 values in the Fibonacci sequence
add_executable(fib_50 fib_50.cc)
target_link_libraries(fib_50 PRIVATE pdmath)
add_test(NAME fib_50 COMMAND fib_50)

# qmc_pi: estimate pi using quasi Monte Carlo
add_executable(qmc_pi qmc_pi.cc)
target_link_libraries(qmc_pi PRIVATE pdmath)
add_test(NAME qmc_pi COMMAND qmc_pi)

# vtk_trig_plot: plot sin, cos, tan with VTK
if(VTK_FOUND)
    add_executable(vtk_trig_plot vtk_trig_plot.cc)
    target_link_libraries(vtk_trig_plot PRIVATE ${VTK_LIBRARIES})
    vtk_module_autoinit(TARGETS vtk_trig_plot MODULES ${VTK_LIBRARIES})
    add_test(NAME vtk_trig_plot COMMAND vtk_trig_plot)
else()
    message(STATUS "Skipping vtk_trig_plot (requires VTK)")
endif()
