cmake_minimum_required(VERSION 3.20)

##
# Findquadmath.cmake
#
# Find module for locating the GCC quadmath library.
#
# The include directory for quadmath.h will be that of the target GCC and
# requires more complex introspection beyond using find_path() or find_file().
# Therefore, the usage requirements only include -lquadmath.
#
# Since the libquadmath version is tied to the GCC version there is no need to
# specify a version in find_package() but it can be done if necessary. If find
# is successful, quadmath_FOUND is TRUE, and the following are defined:
#
#   quadmath_VERSION            libquadmath version matching the GCC version
#   quadmath::quadmath          IMPORTED INTERFACE target for using libquadmath
#

include(FindPackageHandleStandardArgs)

# try-compile a source file that uses an extern libquadmath function
try_compile(
    quadmath_FOUND
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/quadmath.cc
    LINK_LIBRARIES quadmath
    # note: for consistency with the current project
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ${CMAKE_CXX_STANDARD_REQUIRED}
)
# if not found exit early
if(NOT quadmath_FOUND)
    return()
endif()

# version is just the compiler version
set(quadmath_VERSION ${CMAKE_CXX_COMPILER_VERSION})

# create imported INTERFACE target for usage requirements
add_library(quadmath::quadmath IMPORTED INTERFACE)
target_link_libraries(quadmath::quadmath INTERFACE quadmath)

# validate version
find_package_handle_standard_args(
    quadmath
    # note: usually quadmath_VERSION would be handled by VERSION_VAR but we
    # need to have REQUIRED_VARS passed at least one variable name
    REQUIRED_VARS quadmath_VERSION
    VERSION_VAR quadmath_VERSION
    HANDLE_VERSION_RANGE
)
