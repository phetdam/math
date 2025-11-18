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
    target_link_libraries(vtk_normal_plot PRIVATE ${VTK_LIBRARIES} pdmath)
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
# note: pdmath::native_arch doesn't help this go any faster
add_executable(qmc_pi qmc_pi.cc)
target_link_libraries(qmc_pi PRIVATE pdmath)
add_test(NAME qmc_pi COMMAND qmc_pi)

# qmc_pi_avx: estimate pi using AVX2 quasi Monte Carlo
if(PDMATH_HAS_AVX2)
    add_executable(qmc_pi_avx qmc_pi_avx.cc)
    target_link_libraries(qmc_pi_avx PRIVATE pdmath pdmath::native_arch)
    add_test(NAME qmc_pi_avx COMMAND qmc_pi_avx)
else()
    message(STATUS "Skipping qmc_pi_avx (requires AVX2 support)")
endif()

# vtk_trig_plot: plot sin, cos, tan with VTK
if(VTK_FOUND)
    add_executable(vtk_trig_plot vtk_trig_plot.cc)
    target_link_libraries(vtk_trig_plot PRIVATE ${VTK_LIBRARIES} pdmath)
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
    target_link_libraries(vtk_qmc_pi PRIVATE ${VTK_LIBRARIES} pdmath)
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
    target_link_libraries(vtk_fluent_type PRIVATE ${VTK_LIBRARIES} pdmath)
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
    target_link_libraries(vtk_qmc_pi_grid PRIVATE ${VTK_LIBRARIES} pdmath)
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

# cpu_info: print some CPU information
add_executable(cpu_info cpu_info.cc)
target_link_libraries(cpu_info PRIVATE pdmath)
add_test(NAME cpu_info COMMAND cpu_info)

# qmc_pi_grid: write the QMC pi grid points to stdout or a file
add_executable(qmc_pi_grid qmc_pi_grid.cc)
target_link_libraries(qmc_pi_grid PRIVATE pdmath)
# CLI option tests
# -h, --help
add_test(NAME qmc_pi_grid_h COMMAND qmc_pi_grid -h)
add_test(NAME qmc_pi_grid_help COMMAND qmc_pi_grid --help)
set_tests_properties(
    qmc_pi_grid_h
    qmc_pi_grid_help PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage: qmc_pi_grid"
)
# basic tests
add_test(NAME qmc_pi_grid_n6 COMMAND qmc_pi_grid -n 6)
set_tests_properties(qmc_pi_grid_n6 PROPERTIES PASS_REGULAR_EXPRESSION "\t")
add_test(NAME qmc_pi_grid_n4_csv COMMAND qmc_pi_grid -n 4 -f csv)
set_tests_properties(qmc_pi_grid_n4_csv PROPERTIES PASS_REGULAR_EXPRESSION ",")
add_test(
    NAME qmc_pi_grid_n10_csv_out
    COMMAND qmc_pi_grid -n 10 -o qmc_pi_grid_10.csv
)
# -f, --format
add_test(NAME qmc_pi_grid_n4_fbad COMMAND qmc_pi_grid -n 4 -f bad_format_value)
set_tests_properties(
    qmc_pi_grid_n4_fbad PROPERTIES
    PASS_REGULAR_EXPRESSION "Error: Unknown -f, --format value"
)
# bad option
add_test(NAME qmc_pi_grid_n4_badopt COMMAND qmc_pi_grid -n 4 -invalid-option)
set_tests_properties(
    qmc_pi_grid_n4_badopt PROPERTIES
    PASS_REGULAR_EXPRESSION "Error: Unknown option"
)
# missing/unknown file extension with auto format
add_test(NAME qmc_pi_grid_n4_out_noext COMMAND qmc_pi_grid -n 4 -o weird_file)
set_tests_properties(
    qmc_pi_grid_n4_out_noext PROPERTIES
    PASS_REGULAR_EXPRESSION "Error: Cannot deduce output format"
)
add_test(NAME qmc_pi_grid_n4_out_badext COMMAND qmc_pi_grid -n 4 -o bad.badext)
set_tests_properties(
    qmc_pi_grid_n4_out_badext PROPERTIES
    PASS_REGULAR_EXPRESSION "Error: Unknown file format extension"
)
