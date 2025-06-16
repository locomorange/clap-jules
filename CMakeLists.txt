cmake_minimum_required(VERSION 3.16)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Option to enable Skia integration
option(ENABLE_SKIA "Enable Skia graphics library integration" OFF)

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
if(ENABLE_SKIA)
    message(STATUS "Skia integration enabled")
    set(SKIA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia")
    
    if(NOT EXISTS "${SKIA_DIR}")
        message(WARNING "Skia not found at ${SKIA_DIR}. Please run scripts/setup_skia.sh to install Skia. Continuing without Skia integration.")
        set(ENABLE_SKIA OFF)
    endif()
    
    # Find Skia libraries only if directory exists
    if(ENABLE_SKIA)
        find_library(SKIA_LIBRARY
            NAMES skia
            PATHS "${SKIA_DIR}/out/Release"
            NO_DEFAULT_PATH
        )
        
        if(NOT SKIA_LIBRARY)
            message(WARNING "Skia library not found at ${SKIA_DIR}/out/Release. Please run scripts/setup_skia.sh to build Skia. Continuing without Skia integration.")
            set(ENABLE_SKIA OFF)
        endif()
    endif()
    
    # Only proceed with Skia setup if we still have ENABLE_SKIA on
    if(ENABLE_SKIA)
        message(STATUS "Found Skia library: ${SKIA_LIBRARY}")
        
        # Set Skia include directories
        set(SKIA_INCLUDE_DIRS
            "${SKIA_DIR}"
            "${SKIA_DIR}/include"
            "${SKIA_DIR}/include/core"
            "${SKIA_DIR}/include/config"
            "${SKIA_DIR}/include/gpu"
            "${SKIA_DIR}/include/effects"
            "${SKIA_DIR}/include/pathops"
        )
        
        # Create Skia target
        add_library(skia SHARED IMPORTED)
        set_target_properties(skia PROPERTIES
            IMPORTED_LOCATION "${SKIA_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SKIA_INCLUDE_DIRS}"
        )
        
        # Add preprocessor definition
        add_compile_definitions(CLAP_JULES_WITH_SKIA=1)
    endif()
else()
    message(STATUS "Skia integration disabled. Use -DENABLE_SKIA=ON to enable.")
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp plugin_factory.cpp)
target_link_libraries(my_clap_plugin PRIVATE clap)

# Link Skia if enabled
if(ENABLE_SKIA)
    target_link_libraries(my_clap_plugin PRIVATE skia)
    message(STATUS "Linking Skia to plugin")
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
target_link_libraries(my_plugin_tests PRIVATE gtest gtest_main my_clap_plugin clap)

# Add Skia-specific tests if Skia is enabled
if(ENABLE_SKIA)
    add_executable(skia_integration_tests test/test_skia_integration.cpp)
    target_link_libraries(skia_integration_tests PRIVATE gtest gtest_main skia)
    gtest_discover_tests(skia_integration_tests)
endif()

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
