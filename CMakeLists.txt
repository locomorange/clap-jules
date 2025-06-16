cmake_minimum_required(VERSION 3.16)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Graphics support options
option(CLAP_JULES_USE_SKIA "Enable Skia 2D graphics support" OFF)
option(CLAP_JULES_BUILD_GRAPHICS_EXAMPLES "Build graphics examples" ON)

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

# Graphics library setup
if(CLAP_JULES_USE_SKIA)
    message(STATUS "Skia graphics support enabled")
    
    # Try to find Skia using pkg-config or manual paths
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SKIA QUIET skia)
    endif()
    
    if(NOT SKIA_FOUND)
        # Look for Skia in common locations
        find_path(SKIA_INCLUDE_DIR 
            NAMES include/core/SkCanvas.h
            PATHS 
                /usr/local/include/skia
                /usr/include/skia
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/skia
                ${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia
            NO_DEFAULT_PATH
        )
        
        find_library(SKIA_LIBRARY
            NAMES skia
            PATHS
                /usr/local/lib
                /usr/lib
                ${CMAKE_CURRENT_SOURCE_DIR}/libs/skia/out/Release
                ${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia/out/Release
            NO_DEFAULT_PATH
        )
        
        if(SKIA_INCLUDE_DIR AND SKIA_LIBRARY)
            set(SKIA_FOUND TRUE)
            set(SKIA_INCLUDE_DIRS ${SKIA_INCLUDE_DIR})
            set(SKIA_LIBRARIES ${SKIA_LIBRARY})
            message(STATUS "Found Skia: ${SKIA_LIBRARY}")
        else()
            message(WARNING "Skia not found. Please install Skia or set SKIA_INCLUDE_DIR and SKIA_LIBRARY manually.")
            message(STATUS "Falling back to stub graphics implementation")
            set(CLAP_JULES_USE_SKIA OFF)
        endif()
    endif()
else()
    message(STATUS "Using stub graphics implementation (CLAP_JULES_USE_SKIA=OFF)")
endif()

# Graphics library
add_library(clap_jules_graphics STATIC 
    graphics/skia_graphics.cpp
    graphics/skia_graphics.h
)

target_include_directories(clap_jules_graphics PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Ensure position-independent code for static library
set_target_properties(clap_jules_graphics PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(CLAP_JULES_USE_SKIA AND SKIA_FOUND)
    target_compile_definitions(clap_jules_graphics PUBLIC CLAP_JULES_USE_SKIA)
    target_include_directories(clap_jules_graphics PRIVATE ${SKIA_INCLUDE_DIRS})
    target_link_libraries(clap_jules_graphics PRIVATE ${SKIA_LIBRARIES})
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp)
target_link_libraries(my_clap_plugin PRIVATE clap clap_jules_graphics)
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

add_executable(graphics_tests test/test_graphics.cpp)
target_link_libraries(graphics_tests PRIVATE gtest gtest_main clap_jules_graphics)

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
gtest_discover_tests(graphics_tests)

# Graphics examples
if(CLAP_JULES_BUILD_GRAPHICS_EXAMPLES)
    add_executable(graphics_demo examples/graphics_demo.cpp)
    target_link_libraries(graphics_demo PRIVATE clap_jules_graphics)
    
    # GUI test executable (Unix/Linux only - uses dlopen/dlsym)
    if(NOT WIN32)
        add_executable(gui_test examples/gui_test.cpp)
        target_link_libraries(gui_test PRIVATE clap dl)
    endif()
    
    message(STATUS "Graphics demo built: run with './build/graphics_demo'")
    if(NOT WIN32)
        message(STATUS "GUI test built: run with './build/gui_test'")
    endif()
endif()
