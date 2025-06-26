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

# Add GLFW if available and system dependencies are met
set(GLFW_AVAILABLE FALSE)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/CMakeLists.txt")
    # Check if required system dependencies are available
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # Try multiple methods to detect X11 in CI environments
        find_package(X11 QUIET)
        if(X11_FOUND)
            message(STATUS "X11 found via find_package: ${X11_INCLUDE_DIR}")
            set(GLFW_DEPS_FOUND TRUE)
        else()
            # Fallback: Check for X11 headers manually in common locations
            find_path(X11_MANUAL_INCLUDE_DIR 
                NAMES X11/Xlib.h
                PATHS /usr/include /usr/local/include /usr/X11R6/include
                NO_DEFAULT_PATH
            )
            if(X11_MANUAL_INCLUDE_DIR)
                message(STATUS "X11 found manually at: ${X11_MANUAL_INCLUDE_DIR}")
                set(X11_FOUND TRUE)
                set(X11_INCLUDE_DIR ${X11_MANUAL_INCLUDE_DIR})
                set(GLFW_DEPS_FOUND TRUE)
            else()
                # Last resort: Check if libx11-dev package files exist
                if(EXISTS "/usr/include/X11/Xlib.h" OR EXISTS "/usr/local/include/X11/Xlib.h")
                    message(STATUS "X11 headers found in standard locations")
                    set(X11_FOUND TRUE)
                    set(GLFW_DEPS_FOUND TRUE)
                else()
                    message(WARNING "X11 development libraries not found. GLFW GUI support will be disabled.")
                    set(GLFW_DEPS_FOUND FALSE)
                endif()
            endif()
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # Windows should generally have the necessary dependencies
        set(GLFW_DEPS_FOUND TRUE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # macOS should generally have the necessary dependencies
        set(GLFW_DEPS_FOUND TRUE)
    else()
        set(GLFW_DEPS_FOUND FALSE)
    endif()
    
    if(GLFW_DEPS_FOUND)
        message(STATUS "Found GLFW and system dependencies, adding GUI support")
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "Don't build GLFW documentation")
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "Don't build GLFW tests")
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Don't build GLFW examples")
        set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Don't build GLFW with Wayland support")
        add_subdirectory(libs/glfw)
        set(GLFW_AVAILABLE TRUE)
    else()
        message(STATUS "GLFW system dependencies not met, building without GUI support")
    endif()
else()
    message(STATUS "GLFW not found, building without GUI support")
endif()

# Add Skia integration placeholder
# Note: Skia uses GN/Ninja build system. For now, we'll add a placeholder
# for future integration. See docs for building Skia with CMake.
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/skia/include/core/SkCanvas.h")
    message(STATUS "Found Skia headers - integration ready for future implementation")
    # TODO: Add proper Skia CMake integration
    # This would require either:
    # 1. Using Skia's GN build system to generate CMake files
    # 2. Creating a custom CMake wrapper for essential Skia components
    # 3. Using precompiled Skia libraries
    set(SKIA_AVAILABLE TRUE)
else()
    set(SKIA_AVAILABLE FALSE)
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp)
target_link_libraries(my_clap_plugin PRIVATE clap)

# Link GLFW and OpenGL if available
if (GLFW_AVAILABLE AND TARGET glfw)
    target_link_libraries(my_clap_plugin PRIVATE glfw)
    target_compile_definitions(my_clap_plugin PRIVATE HAVE_GLFW=1)
    
    # Link OpenGL on different platforms
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_package(OpenGL QUIET)
        if(OpenGL_FOUND AND TARGET OpenGL::GL)
            target_link_libraries(my_clap_plugin PRIVATE OpenGL::GL)
            # Explicitly link X11 libraries if X11 was found by find_package
            if(X11_FOUND AND TARGET X11::X11)
                target_link_libraries(my_clap_plugin PRIVATE X11::X11)
                message(STATUS "GUI support enabled with GLFW, OpenGL, and explicit X11::X11 linking")
            elseif(X11_FOUND AND X11_LIBRARIES)
                 target_link_libraries(my_clap_plugin PRIVATE ${X11_LIBRARIES})
                 message(STATUS "GUI support enabled with GLFW, OpenGL, and explicit X11_LIBRARIES linking")
            else()
                message(STATUS "GUI support enabled with GLFW and OpenGL (X11 linking might be implicit or missing)")
            endif()
        else()
            # Fallback: Try to link OpenGL manually
            find_library(GL_LIBRARY NAMES GL OpenGL)
            if(GL_LIBRARY)
                target_link_libraries(my_clap_plugin PRIVATE ${GL_LIBRARY})
                # Explicitly link X11 libraries if X11 was found by find_package
                if(X11_FOUND AND TARGET X11::X11)
                    target_link_libraries(my_clap_plugin PRIVATE X11::X11)
                    message(STATUS "GUI support enabled with GLFW, OpenGL (manual), and explicit X11::X11 linking")
                elseif(X11_FOUND AND X11_LIBRARIES)
                     target_link_libraries(my_clap_plugin PRIVATE ${X11_LIBRARIES})
                     message(STATUS "GUI support enabled with GLFW, OpenGL (manual), and explicit X11_LIBRARIES linking")
                else()
                    message(STATUS "GUI support enabled with GLFW and OpenGL (manual linking: ${GL_LIBRARY}, X11 linking might be implicit or missing)")
                endif()
            else()
                # Last resort: Try direct linking for GL
                target_link_libraries(my_clap_plugin PRIVATE GL)
                 # Explicitly link X11 libraries if X11 was found by find_package
                if(X11_FOUND AND TARGET X11::X11)
                    target_link_libraries(my_clap_plugin PRIVATE X11::X11)
                    message(STATUS "GUI support enabled with GLFW, OpenGL (direct GL), and explicit X11::X11 linking")
                elseif(X11_FOUND AND X11_LIBRARIES)
                     target_link_libraries(my_clap_plugin PRIVATE ${X11_LIBRARIES})
                     message(STATUS "GUI support enabled with GLFW, OpenGL (direct GL), and explicit X11_LIBRARIES linking")
                else()
                    message(STATUS "GUI support enabled with GLFW and OpenGL (fallback direct linking GL, X11 linking might be implicit or missing)")
                endif()
            endif()
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # Add Windows system libraries for Win32 API
        target_link_libraries(my_clap_plugin PRIVATE user32 gdi32)
        
        # Multiple approaches to link OpenGL on Windows
        find_library(OPENGL32_LIBRARY NAMES opengl32 OpenGL32)
        if(OPENGL32_LIBRARY)
            target_link_libraries(my_clap_plugin PRIVATE ${OPENGL32_LIBRARY})
            message(STATUS "GUI support enabled with GLFW and OpenGL (${OPENGL32_LIBRARY})")
        else()
            # Try CMake's built-in OpenGL finder
            find_package(OpenGL QUIET)
            if(OpenGL_FOUND AND OPENGL_LIBRARIES)
                target_link_libraries(my_clap_plugin PRIVATE ${OPENGL_LIBRARIES})
                message(STATUS "GUI support enabled with GLFW and OpenGL (via find_package)")
            else()
                # Last resort: direct linking
                target_link_libraries(my_clap_plugin PRIVATE opengl32)
                message(STATUS "GUI support enabled with GLFW and OpenGL (direct linking fallback)")
            endif()
        endif()
        
        # Add Windows-specific compile definitions
        target_compile_definitions(my_clap_plugin PRIVATE _CRT_SECURE_NO_WARNINGS)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        find_library(OPENGL_FRAMEWORK OpenGL)
        if(OPENGL_FRAMEWORK)
            target_link_libraries(my_clap_plugin PRIVATE ${OPENGL_FRAMEWORK})
            message(STATUS "GUI support enabled with GLFW and OpenGL")
        else()
            message(WARNING "OpenGL framework not found. GUI support will be limited.")
            message(STATUS "GUI support enabled with GLFW only (no OpenGL)")
        endif()
    endif()
else()
    message(STATUS "Building without GUI support - GLFW not available")
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

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
