cmake_minimum_required(VERSION 3.22)
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

# Add Brisk library if available (optional, complex dependency management)
option(ENABLE_BRISK "Enable Brisk library integration" OFF)
set(BRISK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk")
if (ENABLE_BRISK AND EXISTS "${BRISK_DIR}/CMakeLists.txt")
    message(STATUS "Found Brisk library, adding as subdirectory")
    # Configure Brisk options
    set(BRISK_TESTS OFF CACHE BOOL "Don't build Brisk tests")
    set(BRISK_EXAMPLES OFF CACHE BOOL "Don't build Brisk examples")
    include(${BRISK_DIR}/cmake/brisk.cmake)
    add_subdirectory(${BRISK_DIR} brisk-bin)
elseif(EXISTS "${BRISK_DIR}/CMakeLists.txt")
    message(STATUS "Brisk library found but disabled by default. Use -DENABLE_BRISK=ON to enable.")
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp plugin_gui.cpp simple_gui.cpp)
target_link_libraries(my_clap_plugin PRIVATE clap)

# Link platform-specific GUI libraries
if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_link_libraries(my_clap_plugin PRIVATE ${X11_LIBRARIES})
    target_include_directories(my_clap_plugin PRIVATE ${X11_INCLUDE_DIR})
    message(STATUS "Linking X11 libraries: ${X11_LIBRARIES}")
endif()

# Link Brisk if enabled
if (ENABLE_BRISK AND TARGET Brisk::Widgets)
    target_link_libraries(my_clap_plugin PRIVATE Brisk::Widgets)
    target_compile_definitions(my_clap_plugin PRIVATE ENABLE_BRISK)
    message(STATUS "Linking Brisk library to plugin")
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

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)

# Add simple GUI test executable
add_executable(test_gui test_gui.cpp simple_gui.cpp)
if (UNIX AND NOT APPLE)
    target_link_libraries(test_gui PRIVATE ${X11_LIBRARIES})
    target_include_directories(test_gui PRIVATE ${X11_INCLUDE_DIR})
endif()

# Add CLAP GUI test executable (disabled on Windows due to complexity)
if (NOT WIN32)
    add_executable(test_clap_gui test_clap_gui.cpp)
    target_link_libraries(test_clap_gui PRIVATE ${CMAKE_DL_LIBS})
    target_include_directories(test_clap_gui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/clap/include")
endif()

# Add Brisk test executable (optional)
if (ENABLE_BRISK AND TARGET Brisk::Widgets)
    add_executable(test_brisk test_brisk.cpp)
    target_link_libraries(test_brisk PRIVATE Brisk::Widgets)
    target_compile_definitions(test_brisk PRIVATE ENABLE_BRISK)
    message(STATUS "Adding Brisk test executable")
endif()
