# Brisk CMake configuration
set(BRISK_FOUND TRUE)
set(BRISK_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")
set(BRISK_LIBRARY_DIR "${CMAKE_CURRENT_LIST_DIR}/lib")

# Create brisk target
if(NOT TARGET brisk)
    add_library(brisk INTERFACE)
    target_include_directories(brisk INTERFACE ${BRISK_INCLUDE_DIR})
    
    # Add platform-specific link libraries when actual brisk is available
    # target_link_libraries(brisk INTERFACE ...)
endif()

message(STATUS "Brisk found: ${BRISK_INCLUDE_DIR}")
