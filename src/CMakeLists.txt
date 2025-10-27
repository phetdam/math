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
    # on Windows, we copy the runtime DLLs into the build directory
    pdmath_copy_runtime_dlls(vtk_normal_plot)
    # note: can add as test since we render offscreen
    # note: VTK will hard abort if there is no display so we use the suggested
    # method of using cmake -E env to run the VTK program
    add_test(
        NAME vtk_normal_plot
        COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:vtk_normal_plot>
    )
    # if using X Windows, the VTK render Window should be using GLX, so if we
    # see this message (no X server) we should skip the test
    if(X11_FOUND)
        set_tests_properties(
            vtk_normal_plot PROPERTIES
            SKIP_REGULAR_EXPRESSION "bad X server"
        )
    endif()
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

# qmc_pi_avx: estimate pi using AVX2 quasi Monte Carlo
# TODO: conditionally enable only for machines with AVX2
add_executable(qmc_pi_avx qmc_pi_avx.cc)
target_link_libraries(qmc_pi_avx PRIVATE pdmath)
# note: these don't really help qmc_pi go any faster
if(MSVC)
    target_compile_options(qmc_pi_avx PRIVATE /arch:AVX2)
else()
    target_compile_options(qmc_pi_avx PRIVATE -march=native)
endif()
add_test(NAME qmc_pi_avx COMMAND qmc_pi_avx)

# vtk_trig_plot: plot sin, cos, tan with VTK
if(VTK_FOUND)
    add_executable(vtk_trig_plot vtk_trig_plot.cc)
    target_link_libraries(vtk_trig_plot PRIVATE ${VTK_LIBRARIES})
    vtk_module_autoinit(TARGETS vtk_trig_plot MODULES ${VTK_LIBRARIES})
    pdmath_copy_runtime_dlls(vtk_trig_plot)
    # note: see above. skip if using GLX and no X server is available
    add_test(
        NAME vtk_trig_plot
        COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:vtk_trig_plot>
    )
    if(X11_FOUND)
        set_tests_properties(
            vtk_trig_plot PROPERTIES
            SKIP_REGULAR_EXPRESSION "bad X server"
        )
    endif()
else()
    message(STATUS "Skipping vtk_trig_plot (requires VTK)")
endif()

# vtk_qmc_pi: plot quasi Monte Carlo estimates of pi
if(VTK_FOUND)
    add_executable(vtk_qmc_pi vtk_qmc_pi.cc)
    target_link_libraries(vtk_qmc_pi PRIVATE ${VTK_LIBRARIES})
    vtk_module_autoinit(TARGETS vtk_qmc_pi MODULES ${VTK_LIBRARIES})
    pdmath_copy_runtime_dlls(vtk_qmc_pi)
    # note: see above. skip if using GLX and no X server is available
    add_test(
        NAME vtk_qmc_pi
        COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:vtk_qmc_pi>
    )
    if(X11_FOUND)
        set_tests_properties(
            vtk_qmc_pi PROPERTIES
            SKIP_REGULAR_EXPRESSION "bad X server"
        )
    endif()
else()
    message(STATUS "Skipping vtk_qmc_cpi (requires VTK)")
endif()

# vtk_fluent_type: print VTK helper pseudo-expression template types
if(VTK_FOUND)
    add_executable(vtk_fluent_type vtk_fluent_type.cc)
    target_link_libraries(vtk_fluent_type PRIVATE ${VTK_LIBRARIES})
    vtk_module_autoinit(TARGETS vtk_fluent_type MODULES ${VTK_LIBRARIES})
    pdmath_copy_runtime_dlls(vtk_fluent_type)
    # note: doesn't need the skipping logic since all expressions involving
    # VTK types are unevaluated (we just use decltype() on them)
    add_test(NAME vtk_fluent_type COMMAND vtk_fluent_type)
else()
    message(STATUS "Skipping vtk_fluent_type (requires VTK)")
endif()

# vtk_qmc_pi_grid: plot the unit circle boundary and the QMC pi grid points
if(VTK_FOUND)
    add_executable(vtk_qmc_pi_grid vtk_qmc_pi_grid.cc)
    target_link_libraries(vtk_qmc_pi_grid PRIVATE ${VTK_LIBRARIES})
    vtk_module_autoinit(TARGETS vtk_qmc_pi_grid MODULES ${VTK_LIBRARIES})
    pdmath_copy_runtime_dlls(vtk_qmc_pi_grid)
    # note: see above. skip if using GLX and no X server is available
    add_test(
        NAME vtk_qmc_pi_grid
        COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:vtk_qmc_pi_grid>
    )
    if(X11_FOUND)
        set_tests_properties(
            vtk_qmc_pi_grid PROPERTIES
            SKIP_REGULAR_EXPRESSION "bad X server"
        )
    endif()
else()
    message(STATUS "Skipping vtk_qmc_pi_grid (requires VTK)")
endif()
