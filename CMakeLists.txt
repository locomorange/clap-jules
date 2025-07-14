# vcpkgツールチェーンファイルをセット
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/libs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

cmake_minimum_required(VERSION 3.22)
project(MyClapPlugin VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find required packages
find_package(PNG REQUIRED)

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

# Optional libraries setup
set(BRISK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/brisk")

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
    if (EXISTS "${BRISK_DIR}/dist/lib" AND EXISTS "${BRISK_DIR}/dist/include")
        # Briskライブラリを正しい順序で収集
        set(BRISK_LIBS
            ${BRISK_DIR}/dist/lib/libbrisk-window.a
            ${BRISK_DIR}/dist/lib/libbrisk-widgets.a
            ${BRISK_DIR}/dist/lib/libbrisk-gui.a
            ${BRISK_DIR}/dist/lib/libbrisk-graphics.a
            ${BRISK_DIR}/dist/lib/libbrisk-core.a
        )
        
        # 存在するライブラリのみを使用
        set(BRISK_LIBRARIES "")
        foreach(lib ${BRISK_LIBS})
            if(EXISTS ${lib})
                list(APPEND BRISK_LIBRARIES ${lib})
            endif()
        endforeach()
        
        if(BRISK_LIBRARIES)
            target_compile_definitions(${target} PRIVATE BRISK_FOUND=1 BRISK_WEBGPU=1)
            target_include_directories(${target} PRIVATE 
                ${BRISK_DIR}/dist/include
                ${BRISK_DIR}/vcpkg_installed/x64-linux/include
            )
            
            # Briskのvcpkgディレクトリを追加
            list(APPEND CMAKE_PREFIX_PATH "${BRISK_DIR}/vcpkg_installed/x64-linux")
            
            # 必要な依存関係
            find_package(Threads REQUIRED)
            find_package(X11 REQUIRED)
            
            # vcpkgからライブラリを直接リンク
            set(BRISK_DEPS "")
            if(EXISTS "${BRISK_DIR}/vcpkg_installed/x64-linux/lib")
                file(GLOB GLFW_LIB "${BRISK_DIR}/vcpkg_installed/x64-linux/lib/libglfw3.a")
                file(GLOB FMT_LIB "${BRISK_DIR}/vcpkg_installed/x64-linux/lib/libfmt.a")
                file(GLOB SPDLOG_LIB "${BRISK_DIR}/vcpkg_installed/x64-linux/lib/libspdlog.a")
                file(GLOB LLVM_LIB "${BRISK_DIR}/vcpkg_installed/x64-linux/lib/libLLVM*.a")
                
                if(GLFW_LIB)
                    list(APPEND BRISK_DEPS ${GLFW_LIB})
                endif()
                if(FMT_LIB)
                    list(APPEND BRISK_DEPS ${FMT_LIB})
                endif()
                if(SPDLOG_LIB)
                    list(APPEND BRISK_DEPS ${SPDLOG_LIB})
                endif()
                if(LLVM_LIB)
                    list(APPEND BRISK_DEPS ${LLVM_LIB})
                endif()
            endif()
            
            # Briskライブラリとその依存関係をリンク（順序が重要）
            target_link_libraries(${target} PRIVATE 
                ${BRISK_LIBRARIES}
                ${BRISK_DEPS}
                Threads::Threads
                ${X11_LIBRARIES}
                ${CMAKE_DL_LIBS}
            )
            
            # 追加のシステムライブラリ
            if(UNIX AND NOT APPLE)
                target_link_libraries(${target} PRIVATE
                    Xi
                    Xrandr
                    Xxf86vm
                    Xinerama
                    Xcursor
                )
            endif()
        endif()
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