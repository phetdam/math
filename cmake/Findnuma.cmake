cmake_minimum_required(VERSION 3.20)

##
# Findnuma.cmake
#
# Find the libnuma Linux NUMA support library.
#
# Since this is a Linux-specific library, not POSIX, we look for it separately.
# On success, numa_FOUND is TRUE, and the following are defined:
#
#   numa_VERSION            libnuma version. If pkg-config cannot locate the
#                           numa package, which is how the precise package
#                           version is determined, the numa.h API version is
#                           queried as a backup, with a warning emitted.
#
#   numa_INCLUDE_DIRS       libnuma include directory
#   numa_LIBRARY            libnuma library path
#   numa::numa              IMPORTED INTERFACE library target for libnuma
#

include(FindPackageHandleStandardArgs)

# locate numa.h
find_path(numa_INCLUDE_DIRS numa.h)
if(NOT numa_INCLUDE_DIRS)
    find_package_handle_standard_args(numa REQUIRED_VARS numa_INCLUDE_DIRS)
    return()
endif()

# locate libnuma library
find_library(numa_LIBRARY numa)
if(NOT numa_LIBRARY)
    find_package_handle_standard_args(numa REQUIRED_VARS numa_LIBRARY)
    return()
endif()

# libnuma is typically packaged with pkg-config
find_package(PkgConfig QUIET)
# note: latter deprecated since CMake 4.2
if(PkgConfig_FOUND OR PKG_CONFIG_FOUND)
    # note: already got header + library so make this call required
    # note: use _numa prefix to avoid conflict with CMake package name
    pkg_check_modules(_numa QUIET REQUIRED numa)
    # set version
    set(numa_VERSION "${_numa_VERSION}")
# if not found just use the API version in the header as a backup
else()
    message(
        WARNING
        "Unable to locate libnuma version using pkg-config. Using API version "
"for a rough estimate"
    )
    file(
        STRINGS "${numa_INCLUDE_DIRS}/numa.h" numa_VERSION
        REGEX "^#define[ \t]+LIBNUMA_API_VERSION[ \t]+[0-9]+.*"
    )
    # should be a single string so transform
    string(
        REGEX REPLACE
        "^#define[ \t]+LIBNUMA_API_VERSION[ \t]+([0-9]+).*" "\\1"
        numa_VERSION "${numa_VERSION}"
    )
    # note: to reflect the approximate nature we aren't setting minor or patch
endif()

# create IMPORTED target for libnuma
add_library(numa::numa IMPORTED INTERFACE)
target_include_directories(numa::numa INTERFACE "${numa_INCLUDE_DIRS}")
target_link_libraries(numa::numa INTERFACE "${numa_LIBRARY}")

# check required variables and verison
find_package_handle_standard_args(
    numa
    # note: library path goes first for benefit of user message
    REQUIRED_VARS numa_LIBRARY numa_INCLUDE_DIRS
    VERSION_VAR numa_VERSION
    HANDLE_VERSION_RANGE
)
