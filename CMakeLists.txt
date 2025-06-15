cmake_minimum_required(VERSION 3.25)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE Type Unspecified; picking Release")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Option to include GUI support (enabled by default)
option(CLAP_INCLUDE_GUI "Include GUI support in the CLAP plugin" TRUE)

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

# Add VSTGUI if available and GUI is enabled
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/vstgui/CMakeLists.txt" AND ${CLAP_INCLUDE_GUI})
    message(STATUS "Including VSTGUI")
    
    # Check platform dependencies before attempting to build VSTGUI
    set(VSTGUI_CAN_BUILD TRUE)
    
    if(UNIX AND NOT APPLE)
        # Linux - Check for X11 and XCB libraries
        find_package(X11 QUIET)
        find_package(PkgConfig QUIET)
        if(PKG_CONFIG_FOUND)
            pkg_check_modules(XCB xcb QUIET)
        endif()
        
        if(NOT X11_FOUND OR NOT PKG_CONFIG_FOUND OR NOT XCB_FOUND)
            message(WARNING "VSTGUI Linux dependencies not found (X11, XCB, pkg-config). GUI will be disabled.")
            set(VSTGUI_CAN_BUILD FALSE)
        endif()
    endif()
    
    if(VSTGUI_CAN_BUILD)
        # Disable standalone components that require additional dependencies
        set(VSTGUI_STANDALONE OFF CACHE BOOL "Disable VSTGUI standalone components")
        set(VSTGUI_TOOLS OFF CACHE BOOL "Disable VSTGUI tools")
        add_subdirectory(libs/vstgui EXCLUDE_FROM_ALL)
        set(VSTGUI_BUILD_ENABLED TRUE)
        
        # Suppress deprecation warnings on Apple platforms
        if (APPLE)
            target_compile_options(vstgui PRIVATE -Wno-deprecated-declarations)
        endif()
    else()
        message(STATUS "VSTGUI dependencies not met - plugin will be built without GUI support")
        set(VSTGUI_BUILD_ENABLED FALSE)
    endif()
else()
    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/vstgui/CMakeLists.txt")
        message(STATUS "VSTGUI not found - plugin will be built without GUI support")
    else()
        message(STATUS "GUI disabled by option - plugin will be built without GUI support")
    endif()
    set(VSTGUI_BUILD_ENABLED FALSE)
endif()

# Add your plugin target here
set(PLUGIN_SOURCES my_plugin.cpp spectrum_analyzer.cpp spectrum_gui.cpp)


# Add GUI sources only if VSTGUI is enabled
if (${CLAP_INCLUDE_GUI} AND VSTGUI_BUILD_ENABLED)
    list(APPEND PLUGIN_SOURCES my_plugin_gui.cpp)
    # Add Linux-specific extensions on Linux
    if(UNIX AND NOT APPLE)
        list(APPEND PLUGIN_SOURCES my_plugin_linux_extensions.cpp)
    endif()
endif()

add_library(my_clap_plugin SHARED ${PLUGIN_SOURCES})
target_link_libraries(my_clap_plugin PRIVATE clap)

# Link VSTGUI if available
if (${CLAP_INCLUDE_GUI} AND VSTGUI_BUILD_ENABLED)
    target_link_libraries(my_clap_plugin PRIVATE vstgui)
    target_include_directories(my_clap_plugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/vstgui)
    target_compile_definitions(my_clap_plugin PRIVATE VSTGUI_ENABLED=1)
    
    # VSTGUI doesn't include the include paths and compile definitions in the library
    # targets like other CMake libraries do, so explicitly include them here
    target_compile_definitions(my_clap_plugin PRIVATE ${VSTGUI_COMPILE_DEFINITIONS})
else()
    target_compile_definitions(my_clap_plugin PRIVATE VSTGUI_ENABLED=0)
endif()

# Platform-specific settings
if(APPLE)
    target_compile_definitions(my_clap_plugin PRIVATE IS_MAC=1)
    target_link_libraries(my_clap_plugin PRIVATE "-framework CoreFoundation" "-framework AppKit" "-framework CoreGraphics")
elseif(UNIX)
    target_compile_definitions(my_clap_plugin PRIVATE IS_LINUX=1)
elseif(WIN32)
    target_compile_definitions(my_clap_plugin PRIVATE IS_WIN=1)
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
target_include_directories(my_plugin_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
