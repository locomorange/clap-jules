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
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Don't build GLFW with Wayland support")
    set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW with X11 support for GUI")
    add_subdirectory(libs/glfw)
endif()

# Add Skia support (optional, requires pre-built Skia)
option(USE_SKIA "Enable Skia graphics support" OFF)
if(USE_SKIA)
    # Look for pre-built Skia
    find_path(SKIA_INCLUDE_DIR 
        NAMES skia/core/SkCanvas.h
        PATHS 
            "${CMAKE_CURRENT_SOURCE_DIR}/libs/skia/include"
            /usr/local/include
            /usr/include
        DOC "Path to Skia include directory"
    )
    
    find_library(SKIA_LIBRARY
        NAMES skia
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/libs/skia/out/Release"
            "${CMAKE_CURRENT_SOURCE_DIR}/libs/skia/out/Debug"
            /usr/local/lib
            /usr/lib
        DOC "Path to Skia library"
    )
    
    if(SKIA_INCLUDE_DIR AND SKIA_LIBRARY)
        message(STATUS "Found Skia: ${SKIA_LIBRARY}")
        add_library(skia INTERFACE)
        target_include_directories(skia INTERFACE ${SKIA_INCLUDE_DIR})
        target_link_libraries(skia INTERFACE ${SKIA_LIBRARY})
    else()
        message(WARNING "Skia not found. Please build Skia first or set SKIA_INCLUDE_DIR and SKIA_LIBRARY. See README for instructions.")
        set(USE_SKIA OFF)
    endif()
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED 
    my_plugin.cpp
    graphics/skia_graphics.cpp
    graphics/win32_renderer.cpp
    graphics/x11_renderer.cpp
)
target_link_libraries(my_clap_plugin PRIVATE clap)

# Add threading support
find_package(Threads REQUIRED)
target_link_libraries(my_clap_plugin PRIVATE Threads::Threads)

# Link with optional dependencies
# GLFW is temporarily disabled in favor of native window embedding
# if (TARGET glfw)
#     target_link_libraries(my_clap_plugin PRIVATE glfw)
#     target_compile_definitions(my_clap_plugin PRIVATE HAVE_GLFW=1)
#     
#     # Link OpenGL for rendering
#     find_package(OpenGL REQUIRED)
#     target_link_libraries(my_clap_plugin PRIVATE ${OPENGL_LIBRARIES})
#     target_include_directories(my_clap_plugin PRIVATE ${OPENGL_INCLUDE_DIRS})
#     
#     message(STATUS "GLFW support enabled with OpenGL")
# endif()

# Add X11 support on Linux
if (UNIX AND NOT APPLE)
    find_package(X11 QUIET)
    if (X11_FOUND)
        target_link_libraries(my_clap_plugin PRIVATE ${X11_LIBRARIES})
        target_include_directories(my_clap_plugin PRIVATE ${X11_INCLUDE_DIR})
        target_compile_definitions(my_clap_plugin PRIVATE HAVE_X11=1)
        message(STATUS "X11 support enabled")
    else()
        message(STATUS "X11 not found, using software rendering only")
    endif()
endif()

if (USE_SKIA AND TARGET skia)
    target_link_libraries(my_clap_plugin PRIVATE skia)
    target_compile_definitions(my_clap_plugin PRIVATE HAVE_SKIA=1)
    message(STATUS "Skia support enabled")
endif()
# Add Windows-specific libraries for Win32 renderer
if (WIN32)
    target_link_libraries(my_clap_plugin PRIVATE user32 gdi32 kernel32)
    target_link_libraries(my_plugin_tests PRIVATE user32 gdi32 kernel32)
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

add_executable(my_plugin_tests 
    test/test_my_plugin.cpp 
    test/test_gui.cpp
    my_plugin.cpp
    graphics/skia_graphics.cpp
    graphics/win32_renderer.cpp
    graphics/x11_renderer.cpp
)
target_link_libraries(my_plugin_tests PRIVATE gtest gtest_main clap Threads::Threads)

# Add include directories for the test target
target_include_directories(my_plugin_tests PRIVATE 
    ${CLAP_PATH}/include
    libs/clap/include
)

# Add GLFW include directories if available
# if (TARGET glfw)
#     target_include_directories(my_plugin_tests PRIVATE libs/glfw/include)
#     # Also link OpenGL to tests
#     find_package(OpenGL REQUIRED)
#     target_link_libraries(my_plugin_tests PRIVATE ${OPENGL_LIBRARIES})
# endif()

# Add compile definitions for graphics libraries to tests
if (TARGET glfw)
    target_compile_definitions(my_plugin_tests PRIVATE HAVE_GLFW=1)
endif()

# Add X11 support to tests on Linux
if (UNIX AND NOT APPLE AND X11_FOUND)
    target_link_libraries(my_plugin_tests PRIVATE ${X11_LIBRARIES})
    target_include_directories(my_plugin_tests PRIVATE ${X11_INCLUDE_DIR})
    target_compile_definitions(my_plugin_tests PRIVATE HAVE_X11=1)
endif()

if (USE_SKIA AND TARGET skia)
    target_compile_definitions(my_plugin_tests PRIVATE HAVE_SKIA=1)
endif()

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)

# Add GUI test executable (only if GLFW is available)
if (TARGET glfw)
    add_executable(test_gui test_gui.cpp)
    target_link_libraries(test_gui PRIVATE my_clap_plugin clap glfw ${OPENGL_LIBRARIES} Threads::Threads)
    target_include_directories(test_gui PRIVATE 
        ${CLAP_PATH}/include
        libs/clap/include
        libs/glfw/include
        ${OPENGL_INCLUDE_DIRS}
    )
    target_compile_definitions(test_gui PRIVATE HAVE_GLFW=1)
    message(STATUS "GUI test executable added")
endif()
