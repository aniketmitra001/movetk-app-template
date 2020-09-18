if(movetk_INCLUDE_DIR)
    # Already in cache, be silent
    set(movetk_FIND_QUIETLY TRUE)
endif()

find_path(movetk_INCLUDE_DIR NAMES movetk)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(movetk DEFAULT_MSG)

mark_as_advanced(movetk_INCLUDE_DIR)
