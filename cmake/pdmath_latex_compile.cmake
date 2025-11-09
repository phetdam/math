cmake_minimum_required(VERSION 3.20)

include_guard(GLOBAL)

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

include(${CMAKE_CURRENT_LIST_DIR}/pdmath_termcolor.cmake)

##
# Run pdfLaTeX in CMAKE_CURRENT_SOURCE_DIR.
#
# This is a convenience function for invoking the same command repeatedly.
#
# Arguments:
#   input                       TeX input relative to CMAKE_CURRENT_SOURCE_DIR
#   [OPTIONS opts...]           pdfLaTeX options
#   [ORDINAL num]               Ordinal to print as part of the compile message
#                               useful for distinguishing separate invocations
#                               of pdfLaTeX from each other
#   [WORKING_DIRECTORY dir]     Working directory for the execute_process call.
#                               If not provided, uses CMAKE_CURRENT_SOURCE_DIR.
#
function(pdmath_invoke_pdflatex input)
    cmake_parse_arguments(ARG "" "ORDINAL;WORKING_DIRECTORY" "OPTIONS" ${ARGN})
    # use empty list if no options
    if(NOT DEFINED ARG_OPTIONS)
        set(ARG_OPTIONS "")
    endif()
    # set working directory
    if(NOT DEFINED ARG_WORKING_DIRECTORY)
        set(ARG_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    # set pdflatex message
    set(pdflatex_msg "pdfTeX compile ${input}")
    if(DEFINED ARG_ORDINAL)
        string(APPEND pdflatex_msg " (${ARG_ORDINAL})")
    endif()
    # set pdflatex command
    set(pdflatex_cmd ${PDFLATEX_COMPILER} ${ARG_OPTIONS} ${input})
    # print message + call pdflatex
    pdmath_message(BOLD_BLUE "${pdflatex_msg}")
    execute_process(
        COMMAND ${pdflatex_cmd}
        WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
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
endfunction()

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
#       -output-directory <directory of input>
#
# There is special handling for old-ish versions of the minted package where
# the -shell-escape option is added for .tex files requiring minted.
#
# Note:
#
# There is no support for writing the TeX intermediate output into the build
# directory given by PDMATH_CURRENT_BINARY_DIR because some packages like
# minted don't play well out-of-the-box with separate source/build layouts. In
# particular, you need to use \usepackage[outputdir=<dir>]{minted} prior to
# minted 3.x, and afterwards, set a different environment variable to tell
# minted where it expects the intermediate .pyg to be created.
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
    # strip extension for BibTeX
    string(REGEX REPLACE "\.tex$" "" input_noext "${input}")
    # regex for a path pattern
    set(path_char "[a-zA-Z0-9_./\\-]")
    # regex for a \bibliography, \input, or \include directive. capture groups
    # are already set for later referencing
    set(dep_regex "[ \t]*\\\\(bibliography|input|include){(${path_char}+)}")
    # scan file for \bibliography, \input, \include commands. these will be
    # additional deps that we will output into the dependency file
    file(STRINGS ${input} tex_lines REGEX "${dep_regex}")
    # remove \bibliography directives from file names and replace with .bib
    list(
        TRANSFORM tex_lines
        REPLACE "[ \t]*\\\\bibliography{(${path_char}+)}" "\\1.bib"
    )
    # remove \input and \include directives and replace with .tex
    list(
        TRANSFORM tex_lines
        REPLACE "[ \t]*\\\\(input|include){(${path_char}+)}" "\\2.tex"
    )
    # build depfile content
    set(depfile_content "${CMAKE_CURRENT_SOURCE_DIR}/${input_noext}.pdf:")
    foreach(dep ${tex_lines})
        string(APPEND depfile_content " \\\n  ${CMAKE_CURRENT_SOURCE_DIR}/${dep}")
    endforeach()
    string(APPEND depfile_content " \\\n  ${CMAKE_CURRENT_SOURCE_DIR}/${input}")
    # generate depfile content
    file(WRITE ${PDMATH_CURRENT_BINARY_DIR}/${input}.d "${depfile_content}\n")
    # special handling for minted, which requires -shell-escape. check if a
    # \usepackage{minted} directive is available
    file(
        STRINGS ${input} minted_lines
        REGEX "[ \t]*\\\\usepackage(\\[.+\\])?{[a-zA-Z0-9_,]*minted[a-zA-Z0-9_,]*}"
    )
    list(LENGTH minted_lines have_minted)
    # pdflatex options
    set(pdflatex_opts -interaction=nonstopmode -halt-on-error)
    if(have_minted)
        list(APPEND pdflatex_opts -shell-escape)
    endif()
    # run pdflatex for the first time
    pdmath_invoke_pdflatex(${input} OPTIONS ${pdflatex_opts} ORDINAL 1)
    # read .aux file to determine if BibTeX needs to be run. that is, we just
    # check for the presence of a single \bibdata command
    file(
        STRINGS ${input_noext}.aux bibdata_lines
        REGEX "[ \t]*\\\\bibdata{${path_char}+}"
    )
    list(LENGTH bibdata_lines bibdata_line_count)
    # ok, at least one \bibdata command, so run BibTeX
    if(bibdata_line_count)
        pdmath_message(BOLD_BLUE "BibTeX compile ${input_noext}.aux")
        set(bibtex_cmd ${BIBTEX_COMPILER} ${input_noext}.aux)
        execute_process(
        # note: no longer necessary since we do TeX compile in the source
        # directory directly. it's just too much hassle to try and work around
        # this deficiency in the TeX build tooling ecosystem.
        #[[==
            # note: on WSL1, for some reason BibTeX might be getting blocked
            # from writing files in the build directory for whatever reason, so
            # we first copy the .aux file into the source directory
            COMMAND ${CMAKE_COMMAND}
                    -E copy ${PDMATH_CURRENT_BINARY_DIR}/${input_noext}.aux
                    ${input_noext}.aux
        #==]]
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
        # accumulate. since the BibTeX command may fail we run these separately
        # note: no longer necessary since we build in the source tree
        #[[==
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
        #==]]
        # check BibTeX command for failure
        if(bibtex_res)
            list(JOIN bibtex_cmd " " bibtex_cmd_str)
            message(
                FATAL_ERROR
                "ERROR: ${bibtex_cmd_str}\n"
"STDERR:\n${bibtex_err}\nSTDOUT:${bibtex_out}"
            )
        endif()
    endif()
    # pdflatex compile (2). this should almost always be used because if you
    # are using an inline bibliography, you still need to re-run once for the
    # .aux file references to be correctly picked up
    pdmath_invoke_pdflatex(${input} OPTIONS ${pdflatex_opts} ORDINAL 2)
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
        pdmath_invoke_pdflatex(${input} OPTIONS ${pdflatex_opts} ORDINAL 3)
    endif()
endfunction()

##
# Compile a TeX file in LaTeX mode using pdflatex.
#
# This runs an optimal sequence of pdflatex/bibtex commands.
#
# See pdmath_latex_compile_impl for details on the pdflatex options. It is
# assumed that PDFLATEX_COMPILER and BIBTEX_COMPILER are defined.
#
# Arguments:
#   input                   TeX input file relative to CMAKE_CURRENT_SOURCE_DIR
#   DEPENDS targets...      Target dependencies. These do *not* affect the
#                           actual PDF generation but simply ensure that the
#                           mentioned targets will also be driven by the dummy
#                           custom target used to drive PDF generation.
#
function(pdmath_latex_compile input)
    # parse options
    # TODO: accept a VERBOSE option that is passed to the script
    cmake_parse_arguments(ARG "VERBOSE" "" "DEPENDS" ${ARGN})
    # strip extension for BibTeX
    string(REGEX REPLACE "\.tex$" "" input_noext "${input}")
    # target for all PDF targets
    if(NOT TARGET pdmath_tex)
        add_custom_target(pdmath_tex ALL COMMENT "Compiling .tex files to .pdf")
    endif()
    # create custom command for PDF from .tex file
    set(pdf_output ${CMAKE_CURRENT_SOURCE_DIR}/${input_noext}.pdf)
    add_custom_command(
        # note: other pdflatex outputs are ignored
        OUTPUT ${pdf_output}
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
    # add custom target for the PDF file so pdmath_tex drives the build
    string(REGEX REPLACE "[\\/.]" "_" pdf_target "pdftex_${input_noext}")
    add_custom_target(
        ${pdf_target}
        # note: no more copy_if_different necessary as build in source tree
        # COMMAND ${CMAKE_COMMAND} -E copy_if_different ${input_noext}.pdf
        #         ${pdf_output}
        DEPENDS ${pdf_output}
    )
    # if target dependencies are provided add them to the PDF target
    #
    # TODO:
    #
    # remove as this doesn't actually affect PDF generation. what we need to do
    # is ensure that -recorder is passed to the pdflatex invocations in
    # pdmath_latex_compile_impl and then after all the pdfTeX and BibTeX
    # invocations we use the .fls file output to construct the .d depfile. we
    # will need to de-duplicate some paths and ensure paths like ./file and
    # changed to just file for propert path de-duplication. this has the
    # downside of including TeX package files in the .d depfile but there is
    # no option like with GCC's -MMD where system stuff is excluded.
    #
    if(ARG_DEPENDS)
        add_dependencies(${pdf_target} ${ARG_DEPENDS})
    endif()
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
