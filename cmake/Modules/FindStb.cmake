# FindStb.cmake
# Finds the STB header-only libraries installed by vcpkg or other means.
#
# Sets the following variables:
# Stb_FOUND          - True if STB headers were found.
# Stb_INCLUDE_DIR    - Directory containing STB headers.
# Stb_LIBRARIES      - Empty, as STB is header-only.

find_path(Stb_INCLUDE_DIR NAMES stb_image.h
    HINTS ENV Stb_DIR
    PATH_SUFFIXES include include/stb stb
    PATHS ${CMAKE_PREFIX_PATH} # Should include vcpkg install paths
          ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include # Explicit vcpkg path
          ${VCPKG_INSTALLED_DIR}/include # Another common vcpkg path pattern
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Stb
    FOUND_VAR Stb_FOUND
    REQUIRED_VARS Stb_INCLUDE_DIR
)

if(Stb_FOUND AND NOT TARGET Stb::stb)
    add_library(Stb::stb INTERFACE IMPORTED)
    target_include_directories(Stb::stb INTERFACE "${Stb_INCLUDE_DIR}")
endif()

mark_as_advanced(Stb_INCLUDE_DIR)
