cmake_minimum_required(VERSION 3.16)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Try to find CLAP automatically, then try to find it in a fixed location.
# This is useful for development, but you may want to remove it for production.
# If you want to use a custom location, you can set CLAP_PATH before running cmake.
# For example: cmake -DCLAP_PATH=/path/to/clap ..
find_package(clap CONFIG)

# Set CLAP_PATH for include directories
if (DEFINED ENV{CLAP_PATH})
    set(CLAP_PATH $ENV{CLAP_PATH})
elseif(DEFINED CLAP_PATH)
    # CLAP_PATH is already set (e.g. by cmake -DCLAP_PATH=...)
else()
    # Try a default location (e.g. a submodule)
    set(CLAP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/clap")
endif()

if (NOT CLAP_FOUND)

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

# Find and link GLFW for GUI support
# Try to find GLFW using find_package first (works on most systems)
find_package(glfw3 CONFIG QUIET)

if(NOT glfw3_FOUND)
    # If not found, try pkg-config (Linux/Unix systems)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLFW3 QUIET glfw3)
    endif()
    
    if(NOT GLFW3_FOUND)
        # If still not found, build GLFW from source using FetchContent
        message(STATUS "GLFW not found, building from source...")
        include(FetchContent)
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG        3.3.10
        )
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glfw)
        set(GLFW_LIBRARIES glfw)
        set(GLFW_INCLUDE_DIRS "")
    else()
        set(GLFW_LIBRARIES ${GLFW3_LIBRARIES})
        set(GLFW_INCLUDE_DIRS ${GLFW3_INCLUDE_DIRS})
    endif()
else()
    set(GLFW_LIBRARIES glfw)
    set(GLFW_INCLUDE_DIRS "")
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp gui.cpp renderer.cpp)
target_link_libraries(my_clap_plugin PRIVATE clap ${GLFW_LIBRARIES} OpenGL::GL)
target_include_directories(my_clap_plugin PRIVATE ${GLFW_INCLUDE_DIRS} ${CLAP_PATH}/include)
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

add_executable(gui_tests test/test_gui.cpp)
target_link_libraries(gui_tests PRIVATE gtest gtest_main my_clap_plugin ${GLFW_LIBRARIES} OpenGL::GL)
target_include_directories(gui_tests PRIVATE ${GLFW_INCLUDE_DIRS} ${CLAP_PATH}/include)

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
gtest_discover_tests(gui_tests)
