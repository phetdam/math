cmake_minimum_required(VERSION ${CMAKE_MINIMUM_REQUIRED_VERSION})

##
# Function to generate a CSV file with the QMC pi estimation grid points.
#
# This will create a custom command that writes a file called
# qmc_pi_grid_<nu>.csv in the current source directory (because TeX doesn't
# work well with a separate binary directory).
#
# Arguments:
#   nu      Number of points to use in one dimension (total sampled is nu x nu)
#
function(pdmath_gen_qmc_pi_grid nu)
    set(output_file qmc_pi_grid_${nu}.csv)
    set(full_output_file ${CMAKE_CURRENT_SOURCE_DIR}/${output_file})
    add_custom_command(
        OUTPUT ${full_output_file}
        COMMAND qmc_pi_grid -n ${nu} -o ${full_output_file}
        DEPENDS qmc_pi_grid
        COMMENT "Generating ${output_file}"
    )
endfunction()

# custom commands + target for generating the tabular data used by qmc_pi.tex
pdmath_gen_qmc_pi_grid(10)
pdmath_gen_qmc_pi_grid(20)
add_custom_target(
    pdmath_qmc_pi_grids
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/qmc_pi_grid_10.csv
            ${CMAKE_CURRENT_SOURCE_DIR}/qmc_pi_grid_20.csv
)

pdmath_latex_compile(bac_returning_math_qs.tex)
pdmath_latex_compile(expectation_normal_cdf.tex)
pdmath_latex_compile(revisiting_black76.tex)
pdmath_latex_compile(qmc_pi.tex DEPENDS pdmath_qmc_pi_grids)
