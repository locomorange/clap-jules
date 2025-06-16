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

# Skia integration (optional)
option(ENABLE_SKIA "Enable Skia graphics support" OFF)
set(SKIA_FOUND FALSE)

if(ENABLE_SKIA)
    # Try to find Skia installation
    if(DEFINED ENV{SKIA_PATH})
        set(SKIA_PATH $ENV{SKIA_PATH})
    elseif(DEFINED SKIA_PATH)
        # SKIA_PATH is already set (e.g. by cmake -DSKIA_PATH=...)
    else()
        # Try a default location
        set(SKIA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/skia")
    endif()

    # Check for Skia headers and libraries
    if(EXISTS "${SKIA_PATH}/include/core/SkCanvas.h")
        message(STATUS "Found Skia headers in ${SKIA_PATH}/include")
        
        # Set up Skia include directories
        set(SKIA_INCLUDE_DIRS 
            "${SKIA_PATH}/include/core"
            "${SKIA_PATH}/include/effects"
            "${SKIA_PATH}/include/gpu"
            "${SKIA_PATH}/include/utils"
            "${SKIA_PATH}/include/config"
        )
        
        # Look for Skia libraries
        if(WIN32)
            set(SKIA_LIB_NAMES "skia.lib" "skia.dll.lib")
            set(SKIA_LIB_DIRS "${SKIA_PATH}/lib" "${SKIA_PATH}/bin")
        else()
            set(SKIA_LIB_NAMES "libskia.a" "libskia.so")
            set(SKIA_LIB_DIRS "${SKIA_PATH}/lib" "${SKIA_PATH}/lib64")
        endif()
        
        foreach(lib_dir ${SKIA_LIB_DIRS})
            foreach(lib_name ${SKIA_LIB_NAMES})
                if(EXISTS "${lib_dir}/${lib_name}")
                    set(SKIA_LIBRARIES "${lib_dir}/${lib_name}")
                    set(SKIA_FOUND TRUE)
                    message(STATUS "Found Skia library: ${SKIA_LIBRARIES}")
                    break()
                endif()
            endforeach()
            if(SKIA_FOUND)
                break()
            endif()
        endforeach()
        
        if(NOT SKIA_FOUND)
            message(WARNING "Skia headers found but no libraries found in ${SKIA_LIB_DIRS}")
        endif()
    else()
        message(WARNING "Skia headers not found in ${SKIA_PATH}/include. Set SKIA_PATH to point to Skia installation directory.")
    endif()
endif()

if(SKIA_FOUND)
    message(STATUS "Skia integration enabled")
    add_compile_definitions(HAVE_SKIA=1)
else()
    message(STATUS "Skia integration disabled")
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp graphics_wrapper.cpp)
target_link_libraries(my_clap_plugin PRIVATE clap)

# Add Skia support to the plugin if available
if(SKIA_FOUND)
    target_include_directories(my_clap_plugin PRIVATE ${SKIA_INCLUDE_DIRS})
    target_link_libraries(my_clap_plugin PRIVATE ${SKIA_LIBRARIES})
    target_compile_definitions(my_clap_plugin PRIVATE HAVE_SKIA=1)
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

add_executable(my_plugin_tests test/test_my_plugin.cpp graphics_wrapper.cpp)
target_link_libraries(my_plugin_tests PRIVATE gtest gtest_main)

# Add Skia support to the test executable if available
if(SKIA_FOUND)
    target_include_directories(my_plugin_tests PRIVATE ${SKIA_INCLUDE_DIRS})
    target_link_libraries(my_plugin_tests PRIVATE ${SKIA_LIBRARIES})
    target_compile_definitions(my_plugin_tests PRIVATE HAVE_SKIA=1)
endif()

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)

# Optional Skia example executable
option(BUILD_SKIA_EXAMPLE "Build the Skia graphics example" OFF)
if(BUILD_SKIA_EXAMPLE)
    add_executable(skia_example skia_example.cpp graphics_wrapper.cpp)
    if(SKIA_FOUND)
        target_include_directories(skia_example PRIVATE ${SKIA_INCLUDE_DIRS})
        target_link_libraries(skia_example PRIVATE ${SKIA_LIBRARIES})
        target_compile_definitions(skia_example PRIVATE HAVE_SKIA=1)
    endif()
    message(STATUS "Skia example executable will be built")
endif()
