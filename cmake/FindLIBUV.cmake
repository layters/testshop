# Standard FIND_PACKAGE module for libuv, sets the following variables:
# LIBUV_FOUND
# LIBUV_INCLUDE_DIRS (only if LIBUV_FOUND)
# LIBUV_LIBRARIES (only if LIBUV_FOUND)


# Try to find the header and the library
find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
find_library(LIBUV_LIBRARY NAMES uv libuv)

# Handle the QUIETLY/REQUIRED arguments, set LIBUV_FOUND if all variables are found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBUV REQUIRED_VARS LIBUV_LIBRARY LIBUV_INCLUDE_DIR)


# Hide internal variables
mark_as_advanced(LIBUV_INCLUDE_DIR LIBUV_LIBRARY)

# Set standard variables
IF(LIBUV_FOUND)
    set(LIBUV_INCLUDE_DIRS "${LIBUV_INCLUDE_DIR}")
    set(LIBUV_LIBRARIES "${LIBUV_LIBRARY}")
ENDIF()
