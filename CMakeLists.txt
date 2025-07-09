cmake_minimum_required(VERSION 3.22)

# Set BRISK_DIR for consistent referencing
set(BRISK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk")

# Set vcpkg toolchain for Brisk prebuilt binaries - MUST be done before project()
if (EXISTS "${BRISK_DIR}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${BRISK_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    # Set vcpkg variables for Brisk prebuilt binaries
    set(VCPKG_INSTALLED_DIR "${BRISK_DIR}/installed" CACHE STRING "")
    set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "")
    set(VCPKG_HOST_TRIPLET "x64-linux" CACHE STRING "")
    message(STATUS "Brisk: Using vcpkg toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
    message(STATUS "Brisk: VCPKG_INSTALLED_DIR: ${VCPKG_INSTALLED_DIR}")
    message(STATUS "Brisk: VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
else()
    message(WARNING "Brisk vcpkg toolchain not found at ${BRISK_DIR}/scripts/buildsystems/vcpkg.cmake")
endif()

project(MyClapPlugin VERSION 0.0.1 LANGUAGES C CXX)



set(CMAKE_CXX_STANDARD 20)
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

# Add Brisk library using proper CMake configuration
if (EXISTS "${BRISK_DIR}/lib/cmake/brisk/BriskConfig.cmake")
    message(STATUS "Found Brisk CMake configuration, loading...")
    find_package(Brisk REQUIRED CONFIG PATHS "${BRISK_DIR}/lib/cmake/brisk" NO_DEFAULT_PATH)
    
    if(Brisk_FOUND)
        message(STATUS "Brisk: Configuration successful with proper targets")
        set(BRISK_FOUND TRUE)
    endif()
else()
    message(WARNING "Brisk not found - run scripts/brisk/setup-brisk.sh to install prebuilt binaries")
    set(BRISK_FOUND FALSE)
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
if (BRISK_FOUND)
    target_link_libraries(my_clap_plugin PRIVATE 
        Brisk::Widgets 
        Brisk::Gui 
        Brisk::Core
        Brisk::Graphics
        Brisk::Window
    )
    message(STATUS "Linking Brisk libraries to my_clap_plugin")
endif()
# On Windows, CLAP plugins are .clap files, but on Linux/macOS they are .so/.dylib
if (WIN32)
  set_target_properties(my_clap_plugin PROPERTIES PREFIX "" SUFFIX ".clap")
else()
  set_target_properties(my_clap_plugin PROPERTIES PREFIX "") # Use default suffix (.so or .dylib)
endif()
set_target_properties(my_clap_plugin PROPERTIES OUTPUT_NAME "MyFirstClapPlugin")

# Additional Brisk setup for plugin if available
if (BRISK_FOUND)
    # Note: brisk_setup_executable is typically for standalone applications
    # For a plugin, we may need different setup, but let's try this if the function exists
    if(COMMAND brisk_setup_executable)
        brisk_setup_executable(my_clap_plugin)
    endif()
endif()


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

add_executable(integration_tests 
    test/test_integration.cpp
    src/brisk_ui_view.cpp
)
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
if (BRISK_FOUND)
    target_link_libraries(my_plugin_tests PRIVATE 
        Brisk::Widgets 
        Brisk::Gui 
        Brisk::Core
        Brisk::Graphics
        Brisk::Window
    )
    target_link_libraries(integration_tests PRIVATE 
        Brisk::Widgets 
        Brisk::Gui 
        Brisk::Core
        Brisk::Graphics
        Brisk::Window
    )
    message(STATUS "Linking Brisk libraries to test targets")
endif()

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
gtest_discover_tests(integration_tests)
