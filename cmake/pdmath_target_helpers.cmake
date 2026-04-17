cmake_minimum_required(VERSION 3.21)

##
# pdmath_target_helpers.cmake
#
# CMake target helpers.
#

include_guard(GLOBAL)

##
# Copy the DLLs required by the target to the target's output directory.
#
# This ensures that any of the DLLs listed in TARGET_RUNTIME_DLLS, which
# requires that a target dependency correctly sets IMPORTED_LOCATION with the
# path of the DLL. On non-Windows systems this does nothing.
#
# Note: If several targets need to copy the same runtime DLLs to the same
# location in a build directory this will cause clobbering on the first copy.
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

##
# Get a list of VTK DLLs for the current build config.
#
# This function provides the list of VTK DLLs copied by the pdmath_vtk_dlls
# target so targets that all share the VTK DLLs on Windows don't need to
# clobber each other by using pdmath_copy_runtime_dlls(). We implement the
# logic in a separate function for variable scoping and due to length of logic.
#
# The list of DLLs will contain generator expressions if a multi-config
# generator is used. No extra quoting is done so if using with
# add_custom_target() or add_custom_command() VERBATIM and COMMAND_EXPAND_LISTS
# options should be used to ensure proper expansion and quoting.
#
# Arguments:
#   var         Variable to write list of DLLs to
#
function(pdmath_get_vtk_dlls var)
    # iterate through VTK_LIBRARIES targets to locate DLL paths
    foreach(vtk_lib ${VTK_LIBRARIES})
        # try IMPORTED_LOCATION first
        get_target_property(vtk_lib_dll ${vtk_lib} IMPORTED_LOCATION)
        # if found then update list of VTK DLLs
        if(vtk_lib_dll)
            list(APPEND vtk_dlls "${vtk_lib_dll}")
        # not found, so try the config-specific IMPORTED_LOCATION flavors
        else()
            # multi-config requires looping through all configurations and
            # gating with generator expressions to prevent overcopying
            if(PDMATH_IS_MULTI_CONFIG)
                # reset vtk_lib_dll to empty string
                set(vtk_lib_dll "")
                # loop configs
                foreach(config ${CMAKE_CONFIGURATION_TYPES})
                    string(TOUPPER ${config} upper_config)
                    get_target_property(
                        vtk_lib_config_dll ${vtk_lib}
                        IMPORTED_LOCATION_${upper_config}
                    )
                    # add DLL gated by generator expression if available
                    if(vtk_lib_config_dll)
                        string(
                            APPEND vtk_lib_dll
                            "$<$<CONFIG:${config}>:${vtk_lib_config_dll}>"
                        )
                    endif()
                endforeach()
            # single-config only has one path
            else()
                string(TOUPPER "${CMAKE_BUILD_TYPE}" upper_config)
                get_target_property(
                    vtk_lib_dll ${vtk_lib}
                    IMPORTED_LOCATION_${upper_config}
                )
            endif()
            # if DLL [generator expression] found, append, else warn
            if(vtk_lib_dll)
                list(APPEND vtk_dlls "${vtk_lib_dll}")
            # multi-config warning
            elseif(PDMPMT_IS_MULTI_CONFIG)
                message(WARNING "Could not locate ${vtk_lib} DLL paths")
            # single-config warning
            else()
                message(
                    WARNING
                    "Could not locate ${vtk_lib} ${CMAKE_BUILD_TYPE} DLL path"
                )
            endif()
        endif()
    endforeach()
    # write list of VTK libraries to variable
    set(${var} ${vtk_dlls} PARENT_SCOPE)
endfunction()
