cmake_minimum_required(VERSION 3.16)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Manual CLAP SDK setup to avoid version conflicts
set(CLAP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/clap")
if (EXISTS "${CLAP_PATH}/include/clap/clap.h")
    message(STATUS "Found CLAP headers in ${CLAP_PATH}/include")
    include_directories(${CLAP_PATH}/include)
    
    # Create a simple clap interface library
    add_library(clap INTERFACE)
    target_include_directories(clap INTERFACE ${CLAP_PATH}/include)
else()
    message(FATAL_ERROR "Could not find CLAP headers in ${CLAP_PATH}/include")
endif()

# Skip clap-helpers for now to avoid build complexity

# Add Boost.DI (header-only library)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include/boost/di.hpp")
    message(STATUS "Found Boost.DI, adding include directory")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include)
    add_compile_definitions(BOOST_DI_CFG_DIAGNOSTICS_LEVEL=0)
endif()

# KFR and Brisk are complex libraries - for now we'll just include headers if available
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/kfr/include")
    message(STATUS "Found KFR headers, adding include directory")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/kfr/include)
endif()

# For now, create a simple mock implementation for Brisk
set(BRISK_MOCK ON CACHE BOOL "Use mock Brisk implementation")

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add your plugin target here
set(PLUGIN_SOURCES
    my_plugin.cpp
    src/plugin/plugin_view_model.cpp
    src/plugin/plugin_window.cpp
    src/brisk/window.cpp
    src/brisk/widgets.cpp
)

# Add platform-specific native window implementation
if(APPLE)
    list(APPEND PLUGIN_SOURCES src/native/native_window.mm)
    set_source_files_properties(src/native/native_window.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
else()
    list(APPEND PLUGIN_SOURCES src/native/native_window.cpp)
endif()

add_library(my_clap_plugin SHARED ${PLUGIN_SOURCES})

target_link_libraries(my_clap_plugin PRIVATE clap)

# Link platform-specific libraries for native window
if(WIN32)
    target_link_libraries(my_clap_plugin PRIVATE user32 gdi32)
elseif(APPLE)
    target_link_libraries(my_clap_plugin PRIVATE "-framework Cocoa")
else()
    target_link_libraries(my_clap_plugin PRIVATE X11)
endif()

# Set C++ standard for the plugin
target_compile_features(my_clap_plugin PRIVATE cxx_std_17)
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
