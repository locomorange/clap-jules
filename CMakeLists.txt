cmake_minimum_required(VERSION 3.16)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
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

# Add KFR and Brisk (C++20 enabled) - requires CMake 3.22+
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.22")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/kfr/CMakeLists.txt")
        message(STATUS "Found KFR, adding subdirectory")
        add_subdirectory(libs/kfr)
    endif()
    
    # TODO: Re-enable Brisk when pack_resource tool dependency is resolved
    # The issue is that libs/brisk/CMakeLists.txt:137 includes compile_resources.cmake
    # which has find_program(PACK_RESOURCE_TOOL NAMES pack_resource REQUIRED)
    # This fails even when BRISK_RESOURCES is OFF because the include is unconditional
    
    # if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk/CMakeLists.txt")
    #     message(STATUS "Found Brisk, setting up dependencies")
    #     
    #     # Set vcpkg triplet for dependency management
    #     if (NOT DEFINED VCPKG_TARGET_TRIPLET)
    #         if (WIN32)
    #             set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "vcpkg triplet")
    #         elseif (APPLE)
    #             set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "vcpkg triplet")
    #         else()
    #             set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "vcpkg triplet")
    #         endif()
    #     endif()
    #     
    #     add_subdirectory(libs/brisk)
    # endif()
    
    message(STATUS "Brisk integration temporarily disabled due to pack_resource tool requirement")
    message(STATUS "MVVM architecture is maintained for future Brisk integration")
else()
    message(STATUS "CMake ${CMAKE_VERSION} < 3.22, skipping Brisk and KFR (require CMake 3.22+)")
endif()

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add your plugin target here
add_library(my_clap_plugin SHARED 
    my_plugin.cpp
    src/plugin/plugin_view_model.cpp
    src/plugin/plugin_window.cpp
)

target_link_libraries(my_clap_plugin PRIVATE clap)

# Link Brisk and KFR if available
if (TARGET brisk)
    target_link_libraries(my_clap_plugin PRIVATE brisk)
    target_compile_definitions(my_clap_plugin PRIVATE BRISK_ENABLED=1)
endif()

if (TARGET kfr)
    target_link_libraries(my_clap_plugin PRIVATE kfr)
    target_compile_definitions(my_clap_plugin PRIVATE KFR_ENABLED=1)
endif()

# Note: Currently no external libraries are linked due to build complexity
# MVVM architecture is maintained for future integration

# Set C++ standard for the plugin
target_compile_features(my_clap_plugin PRIVATE cxx_std_20)
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
