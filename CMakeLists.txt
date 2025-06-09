cmake_minimum_required(VERSION 3.16)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Try to find CLAP automatically, then try to find it in a fixed location.
# This is useful for development, but you may want to remove it for production.
# If you want to use a custom location, you can set CLAP_PATH before running cmake.
# For example: cmake -DCLAP_PATH=/path/to/clap ..
find_package(clap CONFIG)
if (NOT CLAP_FOUND)
    if (DEFINED ENV{CLAP_PATH})
        set(CLAP_PATH $ENV{CLAP_PATH})
    elseif(DEFINED CLAP_PATH)
        # CLAP_PATH is already set (e.g. by cmake -DCLAP_PATH=...)
    else()
        # Try a default location (e.g. a submodule)
        set(CLAP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/clap")
    endif()

    if (EXISTS "${CLAP_PATH}/include/clap/clap.h")
        message(STATUS "Found CLAP headers in ${CLAP_PATH}/include")
    else()
        message(FATAL_ERROR "Could not find CLAP headers in ${CLAP_PATH}/include. Please set CLAP_PATH to the directory containing the CLAP SDK.")
    endif()

    # Add clap as a subdirectory.
    # This will build clap if it's not already built.
    # This also makes the clap targets available to us.
    add_subdirectory(${CLAP_PATH} ${CMAKE_CURRENT_BINARY_DIR}/clap)
endif()

# --- Skia Configuration ---
set(SKIA_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/skia")
set(SKIA_BUILD_DIR "${SKIA_SOURCE_DIR}/out/Release" CACHE PATH "Path to Skia build directory (e.g., skia/out/Release)")

set(SKIA_INCLUDE_DIR_NAMES
    "include/core"
    "include/config"
    "include/codec"
    "include/effects"
    "include/pathops"
    "include/ports"
    "include/utils"
    "include/gpu"
    "modules/skcms/include"
    "modules/skshaper/include"
    "modules/skparagraph/include"
)

set(SKIA_LIB_NAMES
    "skia"
    "skshaper"
    "skparagraph"
    "skcms"
)

# Find Skia Headers
foreach(dir_name ${SKIA_INCLUDE_DIR_NAMES})
  find_path(SKIA_INCLUDE_DIR_${dir_name} NAMES skia.h HINTS "${SKIA_BUILD_DIR}/include/${dir_name}" "${SKIA_SOURCE_DIR}/include/${dir_name}" NO_DEFAULT_PATH)
  if(SKIA_INCLUDE_DIR_${dir_name})
    list(APPEND SKIA_INCLUDE_DIRS "${SKIA_INCLUDE_DIR_${dir_name}}")
  else()
    message(STATUS "Skia include path ${dir_name} not found in ${SKIA_BUILD_DIR}/include/${dir_name} or ${SKIA_SOURCE_DIR}/include/${dir_name}")
  endif()
endforeach()
list(APPEND SKIA_INCLUDE_DIRS "${SKIA_SOURCE_DIR}") # For SkUserConfig.h etc.
list(APPEND SKIA_INCLUDE_DIRS "${SKIA_BUILD_DIR}/gen") # For generated headers

# Find Skia Libraries
foreach(lib_name ${SKIA_LIB_NAMES})
  find_library(SKIA_LIBRARY_${lib_name} NAMES ${lib_name} HINTS "${SKIA_BUILD_DIR}/lib" "${SKIA_BUILD_DIR}/obj" "${SKIA_BUILD_DIR}" NO_DEFAULT_PATH)
  if(SKIA_LIBRARY_${lib_name})
    list(APPEND SKIA_LIBRARIES "${SKIA_LIBRARY_${lib_name}}")
  else()
    message(WARNING "Skia library '${lib_name}' not found in ${SKIA_BUILD_DIR}/lib or /obj.")
  endif()
endforeach()

# Check if Skia is Found and Provide Guidance
if(NOT SKIA_INCLUDE_DIRS OR NOT SKIA_LIBRARIES)
  message(FATAL_ERROR "Skia headers or libraries not found. Please ensure Skia is built within its submodule directory (usually './skia').
Expected Skia build directory: ${SKIA_BUILD_DIR}
Build Skia using commands like:
  cd skia
  python3 tools/git-sync-deps
  # Note: For static library builds (typical for plugins), -DSKIA_DLL is usually not needed.
  # SKIA_STATIC should be defined by Skia's build files when building statically.
  # Ensure your Skia build configuration matches how you intend to link.
  # skia_use_harfbuzz=true is recommended if using skshaper/skparagraph with complex scripts.
  bin/gn gen out/Release --args='is_official_build=true skia_use_system_libraries=false skia_use_libwebp=false skia_use_libjpeg_turbo=false skia_use_libpng=false skia_use_expat=false skia_use_freetype=true skia_enable_fontmgr_custom_directory=false skia_enable_fontmgr_custom_embedded=false skia_enable_fontmgr_custom_empty=true skia_use_harfbuzz=false'
  ninja -C out/Release skia skshaper skparagraph # Add other modules if needed, e.g. sktext skunicode
Adjust SKIA_BUILD_DIR in CMakeCache.txt if your build output is elsewhere.")
else()
  message(STATUS "Found Skia includes: ${SKIA_INCLUDE_DIRS}")
  message(STATUS "Found Skia libraries: ${SKIA_LIBRARIES}")
endif()
# --- End Skia Configuration ---

# --- ImGui Configuration ---
set(IMGUI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
file(GLOB IMGUI_SOURCES
    "${IMGUI_SOURCE_DIR}/imgui.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_draw.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_tables.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_widgets.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_demo.cpp" # Optional, but good for testing
)
set(IMGUI_BACKENDS_DIR "${IMGUI_SOURCE_DIR}/backends")
set(IMGUI_SKIA_BACKEND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/backends/imgui_skia") # New backend directory
# --- End ImGui Configuration ---

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp)
target_sources(my_clap_plugin PRIVATE
    ${IMGUI_SOURCES}
    ${IMGUI_SKIA_BACKEND_DIR}/imgui_impl_skia.cpp # Add new backend source
)
# Link Skia to the plugin
target_include_directories(my_clap_plugin PRIVATE ${SKIA_INCLUDE_DIRS})
# Add ImGui include directories
target_include_directories(my_clap_plugin PRIVATE
    ${IMGUI_SOURCE_DIR}
    ${IMGUI_BACKENDS_DIR}
    ${IMGUI_SKIA_BACKEND_DIR} # Add new backend include dir
    ${CMAKE_CURRENT_SOURCE_DIR} # For my_plugin.h etc.
)
target_link_libraries(my_clap_plugin PRIVATE ${SKIA_LIBRARIES} clap)

# Platform specific libraries for Skia (e.g., FreeType, Fontconfig on Linux)
if (UNIX AND NOT APPLE AND NOT ANDROID) # ANDROID check for toolchain files that might define UNIX
    find_package(PkgConfig QUIET)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(FREETYPE QUIET freetype2)
        pkg_check_modules(FONTCONFIG QUIET fontconfig)
        if (FREETYPE_FOUND)
            target_include_directories(my_clap_plugin PRIVATE ${FREETYPE_INCLUDE_DIRS})
            target_link_libraries(my_clap_plugin PRIVATE ${FREETYPE_LIBRARIES})
            message(STATUS "Linking with system FreeType: ${FREETYPE_LIBRARIES}")
        else()
            message(STATUS "System FreeType not found by PkgConfig. Skia might use its bundled version or fail if not configured to.")
        endif()
        if (FONTCONFIG_FOUND)
            target_include_directories(my_clap_plugin PRIVATE ${FONTCONFIG_INCLUDE_DIRS})
            target_link_libraries(my_clap_plugin PRIVATE ${FONTCONFIG_LIBRARIES})
            message(STATUS "Linking with system Fontconfig: ${FONTCONFIG_LIBRARIES}")
        else()
            message(STATUS "System Fontconfig not found by PkgConfig. Skia might use a fallback or fail if not configured to.")
        endif()
        # Common system libraries often needed by Skia or its dependencies on Linux
        target_link_libraries(my_clap_plugin PRIVATE pthread dl m) # Added 'm' for math library
    else()
        message(WARNING "PkgConfig not found. Cannot automatically find system FreeType/Fontconfig. Linking might fail or Skia might use bundled versions.")
        # Still link common libs if PkgConfig itself is missing
        target_link_libraries(my_clap_plugin PRIVATE pthread dl m)
    endif()
endif()

# On Windows, CLAP plugins are .clap files, but on Linux/macOS they are .so/.dylib
if (WIN32)
  set_target_properties(my_clap_plugin PROPERTIES PREFIX "" SUFFIX ".clap")
else()
  set_target_properties(my_clap_plugin PROPERTIES PREFIX "") # Use default suffix (.so or .dylib)
endif()
set_target_properties(my_clap_plugin PROPERTIES OUTPUT_NAME "MyFirstClapPlugin")


# Install the plugin (optional, but good practice)
# This will install the plugin to <prefix>/lib/clap on Linux/macOS
# and <prefix>/CLAP on Windows.
if(WIN32)
    set(CLAP_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/CLAP")
else()
    set(CLAP_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/clap")
endif()
install(TARGETS my_clap_plugin DESTINATION ${CLAP_INSTALL_DIR})

message(STATUS "CMake setup for CLAP plugin is complete. Plugin target 'MyFirstClapPlugin' added.")

# Google Test integration
add_subdirectory(googletest)
enable_testing()

add_executable(my_plugin_tests test/test_my_plugin.cpp)
target_link_libraries(my_plugin_tests PRIVATE gtest gtest_main my_clap_plugin)

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
