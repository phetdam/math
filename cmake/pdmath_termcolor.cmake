cmake_minimum_required(VERSION 3.17)

include_guard(GLOBAL)

##
# pdmath_termcolor.cmake
#
# Provides ANSI color escape sequences and helpers for colorized messages.
#
# Inspired by https://stackoverflow.com/a/19578320/14227825.
#
# Note: Since Windows Terminal supports virtual terminal sequences by default
# on Windows we check for the WT_SESSION environment variable too.
#

# indicate if on virtual terminal sequences are supported or not
if(WIN32 AND NOT DEFINED ENV{WT_SESSION})
    set(PDMATH_HAVE_VTS 0)
else()
    set(PDMATH_HAVE_VTS 1)
endif()

# ASCII for escape sequence + foreground color reset
string(ASCII 27 PDMATH_ESC)
if(PDMATH_HAVE_VTS)
    set(PDMATH_FG_RESET "${PDMATH_ESC}[m")
else()
    set(PDMATH_FG_RESET "")
endif()

# standard foreground color names and numbers
set(PDMATH_COLOR_NAMES red green yellow blue magenta cyan white)
set(PDMATH_FG_COLOR_VALUES 31 32 33 34 35 36 37)

# define foreground color names and mappings
foreach(name_value IN ZIP_LISTS PDMATH_COLOR_NAMES PDMATH_FG_COLOR_VALUES)
    string(TOUPPER ${name_value_0} ucolor)
    if(PDMATH_HAVE_VTS)
        # standard foreground color
        set(PDMATH_FG_${ucolor} "${PDMATH_ESC}[${name_value_1}m")
        # bold foreground color
        set(PDMATH_FG_BOLD_${ucolor} "${PDMATH_ESC}[1;${name_value_1}m")
    else()
        set(PDMATH_FG_${ucolor} "")
        set(PDMATH_FG_BOLD_${ucolor} "")
    endif()
endforeach()

##
# Emit a colorized CMake message.
#
# This takes a foreground color name, e.g. BLUE or BOLD_BLUE, that controls the
# foreground text color, and forwards remaining arguments to message().
#
# Arguments:
#   color       Foreground color in all caps, e.g. BLUE, BOLD_BLUE
#   ...         Arguments forwarded to message()
#
function(pdmath_message color)
    # color value to use
    set(color_value ${PDMATH_FG_${color}})
    # parse out any mode arguments for message
    set(
        message_modes
            FATAL_ERROR
            SEND_ERROR
            WARNING
            AUTHOR_WARNING
            DEPRECATION
            NOTICE
            STATUS
            VERBOSE
            DEBUG
            TRACE
            CHECK_START
            CHECK_PASS
            CHECK_FAIL
    )
    cmake_parse_arguments(ARG "${message_modes}" "" "" ${ARGN})
    # select message mode (default NOTICE)
    set(message_mode NOTICE)
    foreach(mode ${message_modes})
        if(ARG_${mode})
            set(message_mode ${mode})
            break()
        endif()
    endforeach()
    # call message() with correct arguments
    message(
        ${message_mode}
        "${color_value}" ${ARG_UNPARSED_ARGUMENTS} "${PDMATH_FG_RESET}"
    )
endfunction()
