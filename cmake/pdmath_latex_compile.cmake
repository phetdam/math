cmake_minimum_required(VERSION 3.20)

##
# pdmath_latex_compile.cmake
#
# CMake script for compiling a .tex source into a PDF file.
#
# This is an all-in-one wrapper for the standard pdflatex -> bibtex -> pdflatex
# -> pdflatex compile chain that is needed to compile .tex files with the
# pdfTeX compiler (in LaTeX mode) with BibTeX support. The main implementation
# function should not be called directly (it will be called via cmake -P).
#

##
# Compile a TeX file in LaTeX mode using pdflatex.
#
# This automatically determines how many times pdflatex/bibtex need to be run
# for correct compilation and reference resolution for a TeX document. If the
# .aux file contains a \bibdata entry, the compile stages will be:
#
#   pdflatex -> bibtex -> pdflatex -> pdflatex
#
# With this sequence any BibTeX references are correctly picked up and
# resolved. However, if there is no \bibdata used in the .aux file, BibTeX does
# not need to be run, so the corresponding compile stages will be:
#
#   pdflatex -> pdflatex
#
# The re-run is to ensure that references are correctly resolved when using an
# inline bibliography, e.g. with \begin{thebibliography}. Our .aux file
# scanning ensures we don't needlessly re-run pdflatex or bibtex when not
# necessary, as bibtex will return non-zero the .aux file doesn't have the
# expected \bibdata, \bibstyle commands that it expects.
#
# Furthermore, to support add_custom_command, given a TeX input file input.tex
# in the current source directory, the current build directory will have a
# dependency file input.tex.d that will also include any .bib files used or
# other .tex files that are being included with \include or \input. This
# ensures that rebuilding is optimally triggered.
#
# The following externally defined variables are required:
#
#   PDFLATEX_COMPILER               Path to pdflatex
#   BIBTEX_COMPILER                 Path to bibtex
#   PDMATH_CURRENT_BINARY_DIR       Current binary directory
#
# The default pdflatex options are:
#       -interaction=nonstopmode
#       -halt-on-error
#       -shell-escape
#       -output-directory <directory of input>
#
# Arguments:
#   input           TeX input file relative to CMAKE_CURRENT_SOURCE_DIR
#
function(pdmath_latex_compile_impl input)
    # can only be run in script mode
    if(NOT CMAKE_SCRIPT_MODE_FILE)
        message(
            FATAL_ERROR
            "pdmath_latex_compile_impl can only be run in script mode"
        )
    endif()
    # requires external specification of PDFLATEX_COMPILER + BIBTEX_COMPILER
    if(NOT PDFLATEX_COMPILER)
        message(FATAL_ERROR "PDFLATEX_COMPILER missing")
    endif()
    if(NOT BIBTEX_COMPILER)
        message(FATAL_ERROR "BIBTEX_COMPILER missing")
    endif()
    # requires external specification of CMAKE_CURRENT_BINARY_DIR
    if(NOT PDMATH_CURRENT_BINARY_DIR)
        message(FATAL_ERROR "PDMATH_CURRENT_BINARY_DIR missing")
    endif()
    # pdflatex command
    set(
        pdflatex_cmd
            ${PDFLATEX_COMPILER}
            -interaction=nonstopmode -halt-on-error -shell-escape
            -output-directory ${PDMATH_CURRENT_BINARY_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${input}
    )
    # strip extension for BibTeX
    string(REGEX REPLACE "\.tex$" "" input_noext "${input}")
    # regex for a \bibliography, \input, or \include directive. capture groups
    # are already set for later referencing
    set(dep_regex "[ \t]*\\\\(bibliography|input|include){([a-zA-Z0-9_./\\-]+)}")
    # scan file for \bibliography, \input, \include commands. these will be
    # additional deps that we will output into the dependency file
    file(STRINGS ${input} tex_lines REGEX "${dep_regex}")
    # remove \bibliography directives from file names and replace with .bib
    list(
        TRANSFORM tex_lines
        REPLACE "[ \t]*\\\\bibliography{([a-zA-Z0-9_./\\-]+)}" "\\1.bib"
    )
    # remove \input and \include directives and replace with .tex
    list(
        TRANSFORM tex_lines
        REPLACE "[ \t]*\\\\(input|include){([a-zA-Z0-9_./\\-]+)}" "\\2.tex"
    )
    # build depfile content
    set(depfile_content "${PDMATH_CURRENT_BINARY_DIR}/${input_noext}.pdf:")
    foreach(dep ${tex_lines})
        string(APPEND depfile_content " \\\n  ${CMAKE_CURRENT_SOURCE_DIR}/${dep}")
    endforeach()
    string(APPEND depfile_content " \\\n  ${CMAKE_CURRENT_SOURCE_DIR}/${input}")
    # generate depfile content
    file(WRITE ${PDMATH_CURRENT_BINARY_DIR}/${input}.d "${depfile_content}\n")
    # run pdflatex for the first time
    message(STATUS "pdfTeX compile ${input} (1)")
    execute_process(
        COMMAND ${pdflatex_cmd}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE pdflatex_res
        OUTPUT_VARIABLE pdflatex_out
        ERROR_VARIABLE pdflatex_err
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if(pdflatex_res)
        list(JOIN pdflatex_cmd " " pdflatex_cmd_str)
        # TODO: should just tell people to look at the .log file. this doesn't
        # really display too well and the STDERR will be empty anyways
        message(
            FATAL_ERROR
            "ERROR: ${pdflatex_cmd_str}:\nSTDERR:\n${pdflatex_err}\n"
"STDOUT:${pdflatex_out}"
        )
    endif()
    # read .aux file to determine if BibTeX needs to be run. that is, we just
    # check for the presence of a single \bibdata command
    file(
        STRINGS ${PDMATH_CURRENT_BINARY_DIR}/${input_noext}.aux bibdata_lines
        REGEX "[ \t]*\\\\bibdata{[a-zA-Z0-9_./\\-]+}"
    )
    list(LENGTH bibdata_lines bibdata_line_count)
    # ok, at least one \bibdata command, so run BibTeX
    if(bibdata_line_count)
        message(STATUS "BibTeX compile ${input_noext}.aux")
        set(bibtex_cmd ${BIBTEX_COMPILER} ${input_noext}.aux)
        execute_process(
            # note: on WSL1, for some reason BibTeX might be getting blocked
            # from writing files in the build directory for whatever reason, so
            # we first copy the .aux file into the source directory
            COMMAND ${CMAKE_COMMAND}
                    -E copy ${PDMATH_CURRENT_BINARY_DIR}/${input_noext}.aux
                    ${input_noext}.aux
            COMMAND ${bibtex_cmd}
            # resolve references relative to current directory
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE bibtex_res
            OUTPUT_VARIABLE bibtex_out
            ERROR_VARIABLE bibtex_err
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
        # move .aux + BibTeX outputs back to build directory so they don't
        # accumulate. since the BibTeX command may fail, we run these separately
        execute_process(
            COMMAND ${CMAKE_COMMAND}
                    -E rename ${input_noext}.aux
                    ${PDMATH_CURRENT_BINARY_DIR}/${input_noext}.aux
            COMMAND ${CMAKE_COMMAND}
                    -E rename ${input_noext}.blg
                    ${PDMATH_CURRENT_BINARY_DIR}/${input_noext}.blg
            COMMAND ${CMAKE_COMMAND}
                    -E rename ${input_noext}.bbl
                    ${PDMATH_CURRENT_BINARY_DIR}/${input_noext}.bbl
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        # check BibTeX command for failure
        if(bibtex_res)
            list(JOIN bibtex_cmd " " bibtex_cmd_str)
            message(
                FATAL_ERROR
                "ERROR: ${bibtex_cmd}\nSTDERR:\n${bibtex_err}\nSTDOUT:${bibtex_out}"
            )
        endif()
    endif()
    # pdflatex compile (2). this should almost always be used because if you
    # are using an inline bibliography, you still need to re-run once for the
    # .aux file references to be correctly picked up
    message(STATUS "pdfTeX compile ${input} (2)")
    execute_process(
        COMMAND ${pdflatex_cmd}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE pdflatex_res
        OUTPUT_VARIABLE pdflatex_out
        ERROR_VARIABLE pdflatex_err
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if(pdflatex_res)
        list(JOIN pdflatex_cmd " " pdflatex_cmd_str)
        message(
            FATAL_ERROR
            "ERROR: ${pdflatex_cmd_str}:\nSTDERR:\n${pdflatex_err}\n"
"STDOUT:${pdflatex_out}"
        )
    endif()
    # if we ran BibTeX, we need to run pdflatex again. in general, whenever you
    # are working with bibliographies, you need to do the following:
    #
    #   inline: pdflatex -> pdflatex
    #   BibTeX: pdflatex -> bibtex -> pdflatex -> pdflatex
    #
    # the double pdflatex (or just plain tex/latex) call is necessary for
    # ensuring that references are correctly resolved from the .aux file
    #
    if(bibdata_line_count)
        message(STATUS "pdfTeX compile ${input} (3)")
        execute_process(
            COMMAND ${pdflatex_cmd}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE pdflatex_res
            OUTPUT_VARIABLE pdflatex_out
            ERROR_VARIABLE pdflatex_err
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )
        if(pdflatex_res)
            list(JOIN pdflatex_cmd " " pdflatex_cmd_str)
            message(
                FATAL_ERROR
                "ERROR: ${pdflatex_cmd_str}:\nSTDERR:\n${pdflatex_err}\n"
    "STDOUT:${pdflatex_out}"
            )
        endif()
    endif()
endfunction()

##
# Compile a TeX file in LaTeX mode using pdflatex.
#
# This runs an optimal combination of pdflatex/bibtex commands.
#
# See pdmath_latex_compile_impl for details on the pdflatex options. It is
# assumed that PDFLATEX_COMPILER and BIBTEX_COMPILER are defined.
#
# Arguments:
#   input           TeX input file relative to CMAKE_CURRENT_SOURCE_DIR
#
function(pdmath_latex_compile input)
    # parse options
    # TODO: accept a VERBOSE option that is passed to the script
    cmake_parse_arguments(ARG "VERBOSE" "" "" ${ARGN})
    # strip extension for BibTeX
    string(REGEX REPLACE "\.tex$" "" input_noext "${input}")
    # target for all PDF targets
    if(NOT TARGET pdmath_tex)
        add_custom_target(pdmath_tex ALL COMMENT "Compiling .tex files to .pdf")
    endif()
    # create custom command for .tex file
    add_custom_command(
        # note: other pdflatex outputs are ignored
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${input_noext}.pdf
        COMMAND ${CMAKE_COMMAND}
                -DPDFLATEX_COMPILER=${PDFLATEX_COMPILER}
                -DBIBTEX_COMPILER=${BIBTEX_COMPILER}
                -DPDMATH_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
                -DTEX_INPUT=${input}
                -P ${PROJECT_SOURCE_DIR}/cmake/pdmath_latex_compile.cmake
        DEPFILE ${input}.d
        COMMENT "Generating PDF for ${input}"
        # needs to run in input file directory
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
    # add custom target for the PDF file that copies the PDF back to source dir
    string(REGEX REPLACE "[\\/.]" "_" pdf_target "pdflatex_${input_noext}")
    add_custom_target(
        ${pdf_target}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${input_noext}.pdf
                ${CMAKE_CURRENT_SOURCE_DIR}/${input_noext}.pdf
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${input_noext}.pdf
    )
    # add dependency to top-level target
    add_dependencies(pdmath_tex ${pdf_target})
endfunction()

# if in script mode, run the implementation function
if(CMAKE_SCRIPT_MODE_FILE)
    # TEX_INPUT is required
    if(NOT TEX_INPUT)
        message(FATAL_ERROR "TEX_INPUT missing")
    endif()
    pdmath_latex_compile_impl(${TEX_INPUT})
endif()
