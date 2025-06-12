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

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp)
target_link_libraries(my_clap_plugin PRIVATE clap)
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

# Add clap-host as a submodule for integration testing
# Note: clap-host requires Qt6, so we make it optional
option(BUILD_CLAP_HOST "Build clap-host for integration testing" OFF)

if(BUILD_CLAP_HOST)
    # Check if Qt6 is available
    find_package(Qt6 COMPONENTS Core Widgets QUIET)
    if(Qt6_FOUND)
        message(STATUS "Qt6 found, building clap-host")
        add_subdirectory(clap-host EXCLUDE_FROM_ALL)
        
        # Create a test target that uses clap-host
        add_test(NAME plugin_integration_test_clap_host
                 COMMAND clap-host -p $<TARGET_FILE:my_clap_plugin> --help
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    else()
        message(WARNING "Qt6 not found, skipping clap-host build")
    endif()
endif()

# Create a minimal CLAP plugin loader test for automated testing
add_executable(clap_plugin_test test/clap_plugin_test.cpp)
target_link_libraries(clap_plugin_test PRIVATE clap)
target_include_directories(clap_plugin_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Test that loads and validates the plugin without GUI
add_test(NAME plugin_load_test
         COMMAND clap_plugin_test $<TARGET_FILE:my_clap_plugin>
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
