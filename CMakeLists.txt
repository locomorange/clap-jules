cmake_minimum_required(VERSION 3.16)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable necessary compiler features for modern C++
if(NOT WIN32)
    add_compile_options(-fPIC)
endif()

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

# Add boost-di (header-only dependency injection library)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include/boost/di.hpp")
    message(STATUS "Found boost-di, adding include directory")
    set(BOOST_DI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include")
else()
    message(WARNING "boost-di not found - dependency injection features will be disabled")
endif()

# Add KFR (audio processing library)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/kfr/CMakeLists.txt")
    message(STATUS "Found KFR, configuring...")
    set(KFR_ENABLE_TESTS OFF CACHE BOOL "Don't build KFR tests")
    set(KFR_ENABLE_TOOLS OFF CACHE BOOL "Don't build KFR tools")
    set(KFR_ENABLE_EXAMPLES OFF CACHE BOOL "Don't build KFR examples")
    set(KFR_ENABLE_CAPI_BUILD OFF CACHE BOOL "Don't build KFR C API")
    add_subdirectory(libs/kfr)
else()
    message(WARNING "KFR not found - audio processing features will be limited")
endif()

# Add Brisk (UI library) - temporarily disable until we can resolve build issues
# if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk/CMakeLists.txt")
#     message(STATUS "Found Brisk, configuring...")
#     set(BRISK_EXAMPLES OFF CACHE BOOL "Don't build Brisk examples")
#     set(BRISK_TESTS OFF CACHE BOOL "Don't build Brisk tests")
#     add_subdirectory(libs/brisk)
# else()
#     message(WARNING "Brisk not found - GUI features will be disabled")
# endif()

# For now, let's use brisk as header-only includes to get started
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk/include")
    message(STATUS "Found Brisk includes, adding as header-only")
    set(BRISK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk/include")
else()
    message(WARNING "Brisk not found - GUI features will be disabled")
endif()

# Add your plugin target here  
add_library(my_clap_plugin SHARED 
    my_plugin.cpp
    src/AudioProcessor.cpp
    src/MVVM.cpp
    src/ClapParameterExtension.cpp
)

# Link libraries
target_link_libraries(my_clap_plugin PRIVATE clap)

# Add include directories and link libraries for new dependencies
target_include_directories(my_clap_plugin PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_compile_definitions(my_clap_plugin PRIVATE MY_CLAP_PLUGIN_EXPORTS)

if(BOOST_DI_INCLUDE_DIR)
    target_include_directories(my_clap_plugin PRIVATE "${BOOST_DI_INCLUDE_DIR}")
    message(STATUS "Added boost-di include directory to plugin")
endif()

if(TARGET kfr)
    target_link_libraries(my_clap_plugin PRIVATE kfr kfr_dsp)
    message(STATUS "Linked KFR to plugin")
endif()

if(BRISK_INCLUDE_DIR)
    target_include_directories(my_clap_plugin PRIVATE "${BRISK_INCLUDE_DIR}")
    message(STATUS "Added Brisk include directory to plugin (header-only)")
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

# Add include directories for tests
target_include_directories(my_plugin_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
if(BOOST_DI_INCLUDE_DIR)
    target_include_directories(my_plugin_tests PRIVATE "${BOOST_DI_INCLUDE_DIR}")
endif()
if(TARGET kfr)
    target_link_libraries(my_plugin_tests PRIVATE kfr kfr_dsp)
endif()
if(BRISK_INCLUDE_DIR)
    target_include_directories(my_plugin_tests PRIVATE "${BRISK_INCLUDE_DIR}")
endif()

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
