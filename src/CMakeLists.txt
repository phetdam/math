cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

# fpi: print values of pi for several different floating types
add_executable(fpi fpi.cc)
target_link_libraries(fpi PRIVATE pdmath)
# if using GCC then quadmath is also required
if(quadmath_FOUND)
    target_link_libraries(fpi PRIVATE quadmath::quadmath)
endif()
# simple smoke test
add_test(NAME fpi COMMAND fpi)

##
# Add a CTest test for a VTK application.
#
# This is the same as add_test() except for the fact that the first argument of
# the test command-line is wrapped in a cmake -E env call to handle aborts. In
# particular, when VTK is using the vtkXOpenGLRenderWindow as the concrete type
# for its vtkRenderWindow, if there is no X server running, VTK will log an
# error saying that the X server connection is bad and then abort. We modify
# SKIP_REGULAR_EXPRESSION appropriately to handle this case, relying on if X11
# has been located via find_package(). The reason we do this is because there
# it is hard to directly determine what rendering implementation VTK will use.
# Although we know that typically vtkXOpenGLRenderWindow is the default on
# Linux, due to VTK's auto-init system, it's hard to automatically probe this
# using try_run() since we can't use the auto-init with try_run().
#
# Also note that only VTK applications that render offscreen should be added as
# tests; ones that require interaction and thus block the main thread because
# the GUI event loop is running are obviously unsuitable for batch testing.
#
# Arguments:
#
#   [name_and_cmd]              Test name and test command-line. If specified,
#                               NAME and COMMAND cannot be specified.
#
#   [NAME name]                 Test name. If specified, name_and_cmd cannot be
#                               specified and COMMAND is required.
#
#   [COMMAND cmd [args...]]     Test command-line. If specified, name_and_cmd
#                               cannot be specified and NAME is required.
#
function(pdmath_add_vtk_test)
    # parse arguments
    cmake_parse_arguments(ARG "" "NAME" "COMMAND" ${ARGN})
    # check unparsed arguments for name_and_cmd
    if(ARG_UNPARSED_ARGUMENTS)
        # if length is not 1 this is an invalid call signature
        list(LENGTH ARG_UNPARSED_ARGUMENTS n_unparsed)
        if(NOT n_unparsed EQUAL 1)
            message(FATAL_ERROR "Only one positional argument allowed")
        endif()
        # cannot specify with NAME or COMMAND together
        if(ARG_NAME)
            message(FATAL_ERROR "Cannot specify NAME with name_and_cmd")
        endif()
        if(ARG_COMMAND)
            message(FATAL_ERROR "Cannot specify COMMAND with name_and_cmd")
        endif()
        # update NAME and COMMAND with name_and_cmd
        set(ARG_NAME ${ARG_UNPARSED_ARGUMENTS})
        set(ARG_COMMAND ${ARG_UNPARSED_ARGUMENTS})
    # otherwise NAME, COMMAND required
    else()
        if(NOT ARG_NAME)
            message(FATAL_ERROR "NAME argument required")
        endif()
        if(NOT ARG_COMMAND)
            message(FATAL_ERROR "COMMAND argument required")
        endif()
    endif()
    # pop first element of command
    list(POP_FRONT ARG_COMMAND test_exe)
    # if a target, wrap in a TARGET_FILE generator expression, then update list
    if(TARGET ${test_exe})
        set(test_exe "$<TARGET_FILE:${test_exe}>")
    endif()
    list(PREPEND ARG_COMMAND "${test_exe}")
    # add test
    # note: VTK will hard abort if there is no display so we use the suggested
    # method of using cmake -E env to run the VTK program
    add_test(NAME ${ARG_NAME} COMMAND ${CMAKE_COMMAND} -E env ${ARG_COMMAND})
    # if using X Windows, the VTK render Window should be using GLX, so if we
    # see this message (no X server) we should skip the test
    find_package(X11 QUIET)
    if(X11_FOUND)
        set_tests_properties(
            ${ARG_NAME} PROPERTIES
            SKIP_REGULAR_EXPRESSION "bad X server"
        )
    endif()
endfunction()

# vtk_normal_plot: plot multiple normal PDFs and CDFs with VTK
if(VTK_FOUND)
    add_executable(vtk_normal_plot vtk_normal_plot.cc)
    # note: too many modules to keep track of so just use VTK_LIBRARIES
    target_link_libraries(vtk_normal_plot PRIVATE ${VTK_LIBRARIES} pdmath)
    vtk_module_autoinit(TARGETS vtk_normal_plot MODULES ${VTK_LIBRARIES})
    add_dependencies(vtk_normal_plot pdmath_vtk_dlls)
    # note: can add as test since we render offscreen
    pdmath_add_vtk_test(vtk_normal_plot)
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
    add_dependencies(vtk_trig_plot pdmath_vtk_dlls)
    # note: can add as test since we render offscreen
    pdmath_add_vtk_test(vtk_trig_plot)
else()
    message(STATUS "Skipping vtk_trig_plot (requires VTK)")
endif()

# vtk_qmc_pi: plot quasi Monte Carlo estimates of pi
if(VTK_FOUND)
    add_executable(vtk_qmc_pi vtk_qmc_pi.cc)
    target_link_libraries(vtk_qmc_pi PRIVATE ${VTK_LIBRARIES} pdmath)
    vtk_module_autoinit(TARGETS vtk_qmc_pi MODULES ${VTK_LIBRARIES})
    add_dependencies(vtk_qmc_pi pdmath_vtk_dlls)
    # note: can add as test since we render offscreen
    pdmath_add_vtk_test(vtk_qmc_pi)
else()
    message(STATUS "Skipping vtk_qmc_cpi (requires VTK)")
endif()

# vtk_fluent_type: print VTK helper pseudo-expression template types
if(VTK_FOUND)
    add_executable(vtk_fluent_type vtk_fluent_type.cc)
    target_link_libraries(vtk_fluent_type PRIVATE ${VTK_LIBRARIES} pdmath)
    vtk_module_autoinit(TARGETS vtk_fluent_type MODULES ${VTK_LIBRARIES})
    add_dependencies(vtk_fluent_type pdmath_vtk_dlls)
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
    add_dependencies(vtk_qmc_pi_grid pdmath_vtk_dlls)
    # note: can add as test since we render offscreen
    pdmath_add_vtk_test(vtk_qmc_pi_grid)
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
# -m --method
add_test(NAME qmc_pi_grid_n4_mmt32 COMMAND qmc_pi_grid -n 4 -m mt32)
add_test(NAME qmc_pi_grid_n4_mrect COMMAND qmc_pi_grid -n 4 -m rect)
add_test(NAME qmc_pi_grid_n4_mbad COMMAND qmc_pi_grid -n 4 -m invalid_method)
set_tests_properties(
    qmc_pi_grid_n4_mbad PROPERTIES
    PASS_REGULAR_EXPRESSION "Error: Invalid sampler type"
)
# -fmt32-seed
add_test(
    NAME qmc_pi_grid_n4_mmt32_seed
    COMMAND qmc_pi_grid -n 4 -m mt32 -fmt32-seed 8888
)
add_test(
    NAME qmc_pi_grid_n4_mmt32_no_seed
    COMMAND qmc_pi_grid -n 4 -m mt32 -fmt32-seed
)
set_tests_properties(
    qmc_pi_grid_n4_mmt32_no_seed PROPERTIES
    PASS_REGULAR_EXPRESSION "Missing required argument"
)
add_test(
    NAME qmc_pi_grid_n4_mmt32_bad_seed
    COMMAND qmc_pi_grid -n 4 -m mt32 -fmt32-seed not_a_seed
)
set_tests_properties(
    qmc_pi_grid_n4_mmt32_bad_seed PROPERTIES
    PASS_REGULAR_EXPRESSION "conversion failed"
)

# vtk_akima_plot: plot Akima spline interpolations using the same input data as
# on the MATLAB page https://www.mathworks.com/help/matlab/ref/makima.html
if(VTK_FOUND)
    add_executable(vtk_akima_plot vtk_akima_plot.cc)
    target_link_libraries(vtk_akima_plot PRIVATE ${VTK_LIBRARIES} pdmath)
    vtk_module_autoinit(TARGETS vtk_akima_plot MODULES ${VTK_LIBRARIES})
    add_dependencies(vtk_qmc_pi_grid pdmath_vtk_dlls)
    # note: can add as test since we render offscreen
    pdmath_add_vtk_test(vtk_akima_plot)
else()
    message(STATUS "Skipping vtk_akima_plot (requires VTK)")
endif()
