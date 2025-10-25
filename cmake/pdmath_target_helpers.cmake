cmake_minimum_required(VERSION 3.21)

include_guard(GLOBAL)

##
# pdmath_target_helpers.cmake
#
# CMake target helpers.
#

##
# Copy the DLLs required by the target to the target's output directory.
#
# This ensures that any of the DLLs listed in TARGET_RUNTIME_DLLS, which
# requires that a target dependency correctly sets IMPORTED_LOCATION with thw
# path of the DLL. On non-Windows systems this does nothing.
#
# Arguments:
#   target      Target to copy dependent DLLs for
#
function(pdmath_copy_runtime_dlls target)
    if(WIN32)
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_RUNTIME_DLLS:${target}> $<TARGET_FILE_DIR:${target}>
            COMMENT "Copying runtime DLLs for ${target}"
            COMMAND_EXPAND_LISTS
        )
    endif()
endfunction()
