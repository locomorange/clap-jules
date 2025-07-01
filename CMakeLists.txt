cmake_minimum_required(VERSION 3.22)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES CXX)

# Add custom CMake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find required packages
find_package(Threads REQUIRED)

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
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include")
    message(STATUS "Found boost-di, adding include path")
    set(BOOST_DI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include")
else()
    message(WARNING "boost-di not found, some features may be disabled")
endif()

# Add KFR (DSP library) - but make it optional for now
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/kfr/CMakeLists.txt")
    message(STATUS "Found KFR, adding as subdirectory")
    set(KFR_DISABLE_TESTS ON CACHE BOOL "Disable KFR tests")
    set(KFR_DISABLE_EXAMPLES ON CACHE BOOL "Disable KFR examples")
    set(KFR_DISABLE_TOOLS ON CACHE BOOL "Disable KFR tools")
    set(KFR_STD_COMPLEX OFF CACHE BOOL "Use KFR complex")
    # Note: Brisk may have complex CMake requirements, so we'll add it carefully
    add_subdirectory(libs/kfr)
    set(KFR_AVAILABLE TRUE)
else()
    message(WARNING "KFR not found, audio processing features may be limited")
    set(KFR_AVAILABLE FALSE)
endif()

# Add Brisk (GUI library)
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk/CMakeLists.txt")
    message(STATUS "Found Brisk, adding as subdirectory")
    # Set Brisk options if needed, e.g., disable tests if they cause issues in this context
    set(BRISK_TESTS OFF CACHE BOOL "Disable Brisk tests" FORCE)
    set(BRISK_EXAMPLES OFF CACHE BOOL "Disable Brisk examples" FORCE)
    # On Linux, Brisk might default to WebGPU if available, or D3D11 on Windows.
    # For typical plugin GUI, we might not need WebGPU explicitly unless it's a core requirement.
    # Let's assume default graphics backend is sufficient for now.
    # If ICU is optional and causing issues, it could be disabled:
    # set(BRISK_ICU OFF CACHE BOOL "Disable ICU for Brisk" FORCE)
    add_subdirectory(libs/brisk)
    set(BRISK_AVAILABLE TRUE)
    # Required system libraries for Brisk on Linux (may vary based on Brisk's config)
    # These are often needed for X11, OpenGL, Fontconfig, etc.
    find_package(XKB REQUIRED) # From clap-host, Brisk might need similar
    find_package(Fontconfig REQUIRED)
    find_package(X11 REQUIRED)
    # OpenGL is usually found by Brisk's CMake or via Qt6 if Brisk uses it.
    # If Brisk uses Qt, Qt's dependencies will be handled.
    # If Brisk has its own X11/OpenGL backend, ensure those libs are linked.
else()
    message(WARNING "Brisk not found, GUI will be unavailable or limited")
    set(BRISK_AVAILABLE FALSE)
endif()

# Add your plugin target here
add_library(my_clap_plugin SHARED my_plugin.cpp filter.cpp gui.cpp)
target_link_libraries(my_clap_plugin PRIVATE clap)

# Link Brisk libraries if Brisk is available
if(BRISK_AVAILABLE)
    # Attempt to link with Brisk's main GUI components.
    # Brisk might export targets like Brisk::gui, Brisk::widgets, etc.
    # Or direct library names like brisk-gui, brisk-widgets.
    # Based on brisk's CMakeLists, it creates brisk-gui and brisk-widgets.
    # These likely depend on brisk-core, brisk-graphics, brisk-window.
    # Linking brisk-gui should pull in necessary dependencies.
    target_link_libraries(my_clap_plugin PRIVATE
        brisk-gui # Main GUI logic
        brisk-widgets # Common widgets
        # brisk-window # Windowing system abstraction, likely pulled by gui
        # brisk-graphics # Graphics rendering, likely pulled by gui/window
        # brisk-core # Core utilities, likely pulled by everything else
        # brisk-i18n # If ICU is enabled and used, might be needed
    )
    # Link X11, Fontconfig etc. if Brisk doesn't do it automatically via its targets
    target_link_libraries(my_clap_plugin PRIVATE X11::X11 Fontconfig::Fontconfig Threads::Threads)
    # OpenGL should be handled by Brisk internally if it uses it directly, or by Qt if Brisk uses Qt.
    # For now, assume Brisk's targets handle their own GL needs.
    target_compile_definitions(my_clap_plugin PRIVATE HAVE_BRISK)
endif()

# Add boost-di include if available
if(BOOST_DI_INCLUDE_DIR)
    target_include_directories(my_clap_plugin PRIVATE ${BOOST_DI_INCLUDE_DIR})
    target_compile_definitions(my_clap_plugin PRIVATE HAVE_BOOST_DI)
endif()

# Link KFR for DSP processing if available
if(TARGET kfr AND KFR_AVAILABLE)
    target_link_libraries(my_clap_plugin PRIVATE kfr)
    target_compile_definitions(my_clap_plugin PRIVATE HAVE_KFR)
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

add_executable(my_plugin_tests test/test_my_plugin.cpp filter.cpp)
target_link_libraries(my_plugin_tests PRIVATE gtest gtest_main my_clap_plugin)

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)
