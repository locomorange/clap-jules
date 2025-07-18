# vcpkgツールチェーンファイルをセット
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# Initialize Brisk CMake modules. This must be done before the first project directive.
set(BRISK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk")
# Set Brisk configuration similar to build-brisk-full.sh
# クロスプラットフォーム対応: OSごとにVCPKG_TARGET_TRIPLETを自動設定（CIで上書きも可）
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "Vcpkg target triplet")
    elseif(APPLE)
        set(VCPKG_TARGET_TRIPLET "x64-osx" CACHE STRING "Vcpkg target triplet")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg target triplet")
    endif()
endif()
#set(VCPKG_INSTALLED_DIR "${BRISK_DIR}/vcpkg_installed")
set(VCPKG_MANIFEST_INSTALL ON)
#set(CMAKE_INSTALL_PREFIX dist)
set(VCPKG_INSTALLED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/installed")

if(VCPKG_TARGET_TRIPLET)
    if(WIN32)
        set(PACK_RESOURCE_EXECUTABLE "pack_resource.exe")
    else()
        set(PACK_RESOURCE_EXECUTABLE "pack_resource")
    endif()

    find_program(PACK_RESOURCE_TOOL_PATH
        NAMES ${PACK_RESOURCE_EXECUTABLE}
        HINTS
            "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/pack-resource"
            "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/tools/pack-resource"
    )

    if(PACK_RESOURCE_TOOL_PATH)
        set(PACK_RESOURCE_TOOL ${PACK_RESOURCE_TOOL_PATH} CACHE FILEPATH "Path to pack_resource tool")
    endif()
endif()

# Set pack_resource tool path for Brisk
#set(PACK_RESOURCE_TOOL "${BRISK_DIR}/vcpkg_installed/x64-linux/tools/pack-resource/pack_resource" CACHE FILEPATH "Path to pack_resource tool")
include(${BRISK_DIR}/cmake/brisk.cmake)

cmake_minimum_required(VERSION 3.22)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find required packages

# Find CLAP
if (DEFINED ENV{CLAP_PATH})
    set(CLAP_PATH $ENV{CLAP_PATH})
elseif(NOT DEFINED CLAP_PATH)
    set(CLAP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/clap")
endif()

if (EXISTS "${CLAP_PATH}/include/clap/clap.h")
    message(STATUS "Found CLAP headers in ${CLAP_PATH}/include")
    add_subdirectory(${CLAP_PATH} ${CMAKE_CURRENT_BINARY_DIR}/clap)
else()
    message(FATAL_ERROR "Could not find CLAP headers")
endif()

# Include Brisk source. This must be done after the first project directive.
if (NOT _BRISK_INCLUDE_DIR)
    find_package(Brisk CONFIG REQUIRED)
endif ()
#add_subdirectory(${BRISK_DIR} brisk-bin)
brisk_setup()

# Helper function to setup common target properties
function(setup_target_deps target)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    
    # boost-di (header-only)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include/boost/di.hpp")
        target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs/boost-di/include")
    endif()
    
    # KFR
    if (TARGET kfr)
        target_link_libraries(${target} PRIVATE kfr)
    endif()
    
    # Brisk
    if (TARGET brisk-widgets)
        target_compile_definitions(${target} PRIVATE BRISK_FOUND=1 BRISK_WEBGPU=1)
        target_link_libraries(${target} PRIVATE 
            brisk-widgets
            brisk-window
            brisk-gui
            brisk-graphics
            brisk-core
        )
    endif()
endfunction()

# Add clap-helpers if available
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/clap-helpers/CMakeLists.txt")
    set(CLAP_HELPERS_BUILD_TESTS OFF CACHE BOOL "Don't build clap-helpers tests")
    add_subdirectory(libs/clap-helpers)
endif()

# Add KFR library
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/kfr/CMakeLists.txt")
    set(KFR_ENABLE_TESTS OFF CACHE BOOL "Don't build KFR tests")
    set(KFR_ENABLE_EXAMPLES OFF CACHE BOOL "Don't build KFR examples")
    set(KFR_ENABLE_TOOLS OFF CACHE BOOL "Don't build KFR tools")
    add_subdirectory(libs/kfr)
endif()

# Add plugin target
add_library(my_clap_plugin SHARED 
    my_plugin.cpp
    src/plugin_model.h
    src/audio_processor.h  
    src/plugin_viewmodel.h
)

# Check for Brisk integration sources
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/brisk_clap_integration.h")
    target_sources(my_clap_plugin PRIVATE
        src/brisk_clap_integration.h
        src/brisk_clap_integration.cpp
    )
endif()

# Basic linking and setup
target_link_libraries(my_clap_plugin PRIVATE clap)
setup_target_deps(my_clap_plugin)

# Add Brisk resources
brisk_bundle_resources(my_clap_plugin)

# Set plugin properties
if (WIN32)
    set_target_properties(my_clap_plugin PROPERTIES 
        PREFIX "" 
        SUFFIX ".clap"
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    )
else()
    set_target_properties(my_clap_plugin PROPERTIES PREFIX "")
endif()
set_target_properties(my_clap_plugin PROPERTIES OUTPUT_NAME "MyFirstClapPlugin")

# Install
if(WIN32)
    set(CLAP_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/CLAP")
else()
    set(CLAP_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/clap")
endif()
install(TARGETS my_clap_plugin DESTINATION ${CLAP_INSTALL_DIR})

# Tests
add_subdirectory(libs/googletest)
enable_testing()

add_executable(my_plugin_tests test/test_my_plugin.cpp)
target_link_libraries(my_plugin_tests PRIVATE gtest gtest_main my_clap_plugin)
setup_target_deps(my_plugin_tests)

include(GoogleTest)
gtest_discover_tests(my_plugin_tests)

message(STATUS "CMake configuration complete")