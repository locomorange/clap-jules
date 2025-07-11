cmake_minimum_required(VERSION 3.19)
project(flutter_ui)

# Set the target architecture for Linux desktop build
set(FLUTTER_TARGET_PLATFORM linux-x64)

# Find Flutter installation
find_program(FLUTTER_EXECUTABLE flutter PATHS $ENV{PATH})

if(NOT FLUTTER_EXECUTABLE)
    message(FATAL_ERROR "Flutter executable not found. Please install Flutter and add it to PATH.")
endif()

# Configure Flutter build
set(FLUTTER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/flutter_build")
set(FLUTTER_BUNDLE_DIR "${FLUTTER_BUILD_DIR}/linux/x64/release/bundle")

# Custom target to build Flutter app
add_custom_target(flutter_build
    COMMAND ${FLUTTER_EXECUTABLE} build linux --release
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Flutter application"
)

# Install Flutter bundle (optional)
install(DIRECTORY ${FLUTTER_BUNDLE_DIR}/
    DESTINATION flutter
    OPTIONAL
)