cmake_minimum_required(VERSION 3.16)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Windows-specific compiler settings
if(WIN32 AND MSVC)
    # Disable MSVC warnings for standard library functions
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # Enable proper C++ exception handling
    add_compile_options(/EHsc)
    # Disable specific warnings that might cause build failures
    add_compile_options(/wd4100)  # unreferenced formal parameter
    add_compile_options(/wd4996)  # deprecated functions
    add_compile_options(/wd4251)  # DLL export warnings
    # Ensure we're using the correct calling convention
    add_compile_options(/Zc:__cplusplus)
    # Help with symbol export on Windows
    add_compile_definitions(CLAP_EXPORT_SYMBOLS)
endif()

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
        set(CLAP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/clap")
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

# Add clap-helpers if available
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/clap-helpers/CMakeLists.txt")
    message(STATUS "Found clap-helpers, adding as subdirectory")
    set(CLAP_HELPERS_BUILD_TESTS OFF CACHE BOOL "Don't build clap-helpers tests")
    set(CLAP_HELPERS_NO_CLAP_IS_FATAL OFF CACHE BOOL "Don't make missing clap fatal for clap-helpers")
    add_subdirectory(libs/clap-helpers)
endif()

# Skia graphics library integration
option(ENABLE_SKIA "Enable Skia graphics library support" OFF)

if(ENABLE_SKIA)
    # Try to find Skia in various locations
    set(SKIA_SEARCH_PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/skia"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia"
        "/usr/local/skia"
        "/opt/skia"
    )

    foreach(SKIA_PATH ${SKIA_SEARCH_PATHS})
        if(EXISTS "${SKIA_PATH}/include/core/SkCanvas.h")
            message(STATUS "Found Skia headers in ${SKIA_PATH}")
            set(SKIA_FOUND TRUE)
            set(SKIA_INCLUDE_DIR "${SKIA_PATH}/include")
            
            # Look for Skia library files
            find_library(SKIA_LIBRARY
                NAMES skia libskia
                PATHS "${SKIA_PATH}/out/Release" "${SKIA_PATH}/out/Static" "${SKIA_PATH}/lib"
                NO_DEFAULT_PATH
            )
            
            if(SKIA_LIBRARY)
                message(STATUS "Found Skia library: ${SKIA_LIBRARY}")
                set(HAVE_SKIA TRUE)
            else()
                message(STATUS "Skia headers found but library not built. Please build Skia first.")
                message(STATUS "  cd ${SKIA_PATH}")
                message(STATUS "  python tools/git-sync-deps")
                message(STATUS "  bin/gn gen out/Release --args='is_debug=false'")
                message(STATUS "  ninja -C out/Release")
            endif()
            break()
        endif()
    endforeach()

    if(NOT SKIA_FOUND)
        message(STATUS "Skia not found. Graphics rendering will use basic implementation.")
        message(STATUS "To enable Skia support:")
        message(STATUS "  1. Ensure Skia is available as a git submodule in libs/skia")
        message(STATUS "  2. Build Skia following the instructions in its documentation")
        message(STATUS "  3. Re-run cmake with -DENABLE_SKIA=ON")
    endif()
else()
    message(STATUS "Skia support disabled. Use -DENABLE_SKIA=ON to enable.")
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp graphics_renderer.cpp)
target_link_libraries(my_clap_plugin PRIVATE clap)

# Configure Skia if available
if(HAVE_SKIA)
    target_include_directories(my_clap_plugin PRIVATE ${SKIA_INCLUDE_DIR})
    target_link_libraries(my_clap_plugin PRIVATE ${SKIA_LIBRARY})
    target_compile_definitions(my_clap_plugin PRIVATE HAVE_SKIA=1)
    message(STATUS "Skia integration enabled for plugin")
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
add_subdirectory(libs/googletest)
enable_testing()

add_executable(my_plugin_tests test/test_my_plugin.cpp)
target_link_libraries(my_plugin_tests PRIVATE gtest gtest_main my_clap_plugin)
target_include_directories(my_plugin_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add CLAP include directories to the test target
if(CLAP_FOUND)
    target_link_libraries(my_plugin_tests PRIVATE clap)
else()
    target_include_directories(my_plugin_tests PRIVATE ${CLAP_PATH}/include)
endif()

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
