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

# Add GLFW if available
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/CMakeLists.txt")
    message(STATUS "Found GLFW, adding as subdirectory")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Don't build GLFW docs")
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Don't build GLFW tests")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Don't build GLFW examples")
    
    # Platform-specific GLFW settings
    if(UNIX AND NOT APPLE)
        # Linux-specific settings
        set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Don't build GLFW Wayland support")
        set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW X11 support")
    endif()
    
    add_subdirectory(libs/glfw)
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED 
    my_plugin.cpp
    src/skia_renderer.cpp
)
target_link_libraries(my_clap_plugin PRIVATE clap)

# Include source directory
target_include_directories(my_clap_plugin PRIVATE src)

# Check for system Skia
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SKIA QUIET skia)
    if(SKIA_FOUND)
        message(STATUS "Found system Skia, enabling Skia support")
        target_compile_definitions(my_clap_plugin PRIVATE CLAP_JULES_HAS_SKIA=1)
        target_link_libraries(my_clap_plugin PRIVATE ${SKIA_LIBRARIES})
        target_include_directories(my_clap_plugin PRIVATE ${SKIA_INCLUDE_DIRS})
        target_compile_options(my_clap_plugin PRIVATE ${SKIA_CFLAGS_OTHER})
    else()
        message(STATUS "System Skia not found, Skia support disabled")
        message(STATUS "To enable Skia support, install Skia development packages or set SKIA_ROOT")
    endif()
else()
    message(STATUS "PkgConfig not found, Skia detection disabled")
endif()

# Alternative: Check for custom Skia installation
if(DEFINED SKIA_ROOT)
    if(EXISTS "${SKIA_ROOT}/include/core/SkSurface.h")
        message(STATUS "Found Skia at ${SKIA_ROOT}, enabling Skia support")
        target_compile_definitions(my_clap_plugin PRIVATE CLAP_JULES_HAS_SKIA=1)
        target_include_directories(my_clap_plugin PRIVATE "${SKIA_ROOT}/include")
        
        # Link Skia libraries
        if(WIN32)
            target_link_libraries(my_clap_plugin PRIVATE "${SKIA_ROOT}/lib/skia.lib")
        else()
            target_link_libraries(my_clap_plugin PRIVATE "${SKIA_ROOT}/lib/libskia.a")
        endif()
    endif()
endif()

# Link GLFW if available
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/CMakeLists.txt")
    target_link_libraries(my_clap_plugin PRIVATE glfw)
    target_compile_definitions(my_clap_plugin PRIVATE CLAP_JULES_HAS_GLFW=1)
    
    # Find and link OpenGL
    find_package(OpenGL REQUIRED)
    target_link_libraries(my_clap_plugin PRIVATE OpenGL::GL)
    
    # Platform-specific libraries
    if(WIN32)
        # Windows specific libraries (OpenGL::GL already linked above)
    elseif(APPLE)
        # macOS specific libraries
        find_library(COCOA_LIBRARY Cocoa)
        find_library(IOKIT_LIBRARY IOKit)
        find_library(COREVIDEO_LIBRARY CoreVideo)
        if(COCOA_LIBRARY AND IOKIT_LIBRARY AND COREVIDEO_LIBRARY)
            target_link_libraries(my_clap_plugin PRIVATE ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
            message(STATUS "Found all required macOS frameworks")
        else()
            message(WARNING "Some macOS frameworks not found - GUI may not work properly")
            message(STATUS "COCOA_LIBRARY: ${COCOA_LIBRARY}")
            message(STATUS "IOKIT_LIBRARY: ${IOKIT_LIBRARY}")
            message(STATUS "COREVIDEO_LIBRARY: ${COREVIDEO_LIBRARY}")
        endif()
    else()
        # Linux specific libraries
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(X11 REQUIRED x11)
            target_link_libraries(my_clap_plugin PRIVATE ${X11_LIBRARIES})
            target_include_directories(my_clap_plugin PRIVATE ${X11_INCLUDE_DIRS})
        else()
            message(FATAL_ERROR "PkgConfig is required for Linux builds to find X11 libraries")
        endif()
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

# Google Test integration (optional for testing)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/googletest/CMakeLists.txt")
    message(STATUS "Found Google Test, adding test support")
    add_subdirectory(libs/googletest)
    enable_testing()

    add_executable(my_plugin_tests test/test_my_plugin.cpp)
    target_link_libraries(my_plugin_tests PRIVATE gtest gtest_main my_clap_plugin)

    include(GoogleTest)
    gtest_discover_tests(my_plugin_tests)
else()
    message(STATUS "Google Test not found, skipping test build")
endif()
