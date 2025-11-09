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
    set(output_file ${CMAKE_CURRENT_SOURCE_DIR}/qmc_pi_grid_${nu}.csv)
    add_custom_command(
        OUTPUT ${output_file}
        COMMAND qmc_pi_grid -n ${nu} -o ${output_file}
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
# FIXME: pdmath_qmc_pi_grids doesn't actually directly affect PDF generation as
# it is hung on the driving dummy target and isn't a dependency of the PDF
# generation rule. see pdmath_latex_compile.cmake for details; we can just have
# pdmath_qmc_pi_grids be an ALL target and the pdfTeX -recorder option will
# ensure that the .csv files are in the .fls we use to construct a .d file.
pdmath_latex_compile(qmc_pi.tex DEPENDS pdmath_qmc_pi_grids)
