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

# Add boost-di (header-only library)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include/boost/di.hpp")
    message(STATUS "Found boost-di, adding include directory")
    set(BOOST_DI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include")
else()
    message(WARNING "boost-di not found")
endif()

# Add KFR library
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/kfr/CMakeLists.txt")
    message(STATUS "Found KFR, adding as subdirectory")
    set(KFR_ENABLE_TESTS OFF CACHE BOOL "Don't build KFR tests")
    set(KFR_ENABLE_EXAMPLES OFF CACHE BOOL "Don't build KFR examples")
    set(KFR_ENABLE_TOOLS OFF CACHE BOOL "Don't build KFR tools")
    add_subdirectory(libs/kfr)
else()
    message(WARNING "KFR not found")
endif()

# Add Brisk library
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk/include/brisk/brisk.h")
    message(STATUS "Found Brisk, adding include directory")
    set(BRISK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk/include")
    
    # Create brisk interface target (header-only placeholder)
    add_library(brisk INTERFACE)
    target_include_directories(brisk INTERFACE ${BRISK_INCLUDE_DIR})
    
    # Add compiler definitions to handle brisk as header-only
    target_compile_definitions(brisk INTERFACE BRISK_HEADER_ONLY=1)
else()
    message(WARNING "Brisk not found")
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED 
    my_plugin.cpp
    src/plugin_model.h
    src/audio_processor.h  
    src/plugin_viewmodel.h
    src/brisk_ui_view.h
    src/brisk_ui_view.cpp
)
target_link_libraries(my_clap_plugin PRIVATE clap)

# Add include directories for our source files
target_include_directories(my_clap_plugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link boost-di (header-only)
if (DEFINED BOOST_DI_INCLUDE_DIR)
    target_include_directories(my_clap_plugin PRIVATE ${BOOST_DI_INCLUDE_DIR})
endif()

# Link KFR if available  
if (TARGET kfr)
    target_link_libraries(my_clap_plugin PRIVATE kfr)
endif()

# Link Brisk if available
if (TARGET brisk)
    target_link_libraries(my_clap_plugin PRIVATE brisk)
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

add_executable(integration_tests test/test_integration.cpp)
target_link_libraries(integration_tests PRIVATE gtest gtest_main my_clap_plugin)

# Add include directories for tests
target_include_directories(my_plugin_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(integration_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (DEFINED BOOST_DI_INCLUDE_DIR)
    target_include_directories(my_plugin_tests PRIVATE ${BOOST_DI_INCLUDE_DIR})
    target_include_directories(integration_tests PRIVATE ${BOOST_DI_INCLUDE_DIR})
endif()

# Link KFR for tests if available  
if (TARGET kfr)
    target_link_libraries(my_plugin_tests PRIVATE kfr)
    target_link_libraries(integration_tests PRIVATE kfr)
endif()

# Link Brisk for tests if available
if (TARGET brisk)
    target_link_libraries(my_plugin_tests PRIVATE brisk)
    target_link_libraries(integration_tests PRIVATE brisk)
endif()

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
gtest_discover_tests(integration_tests)
