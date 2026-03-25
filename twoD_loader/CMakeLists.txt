# Simple CMakeLists.txt for TwoDLookupTable library
cmake_minimum_required(VERSION 3.8...3.18)
project(TwoDLookupTable)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ROOT
find_package(ROOT REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ROOT_INCLUDE_DIRS}
)

# Source files
set(SOURCES src/TwoDLookupTable.cc)
set(HEADERS include/TwoDLookupTable.hh)

# Create the static library
add_library(TwoDLookupTableLib STATIC ${SOURCES})
target_link_libraries(TwoDLookupTableLib ${ROOT_LIBRARIES})

# Export variables for parent project
set(TWOD_LOADER_LIBRARY TwoDLookupTableLib PARENT_SCOPE)
set(TWOD_LOADER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)

# Optional: Create standalone test executable (only if building this project directly)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # This is a standalone build
    add_executable(test_2d_loader test_2d_loader.cc)
    target_link_libraries(test_2d_loader TwoDLookupTableLib ${ROOT_LIBRARIES})
    
    # Copy data files to build directory for testing
    file(COPY dataPDE DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    
    message(STATUS "TwoDLookupTable standalone build configured:")
    message(STATUS "  Library: ${SOURCES}")
    message(STATUS "  Test: test_2d_loader")
    message(STATUS "  ROOT: ${ROOT_LIBRARIES}")
endif()

# Create the test executable
add_executable(test_2d_loader test_2d_loader.cc)
target_link_libraries(test_2d_loader TwoDLookupTableLib ${ROOT_LIBRARIES})

# Copy data files to build directory for testing
file(COPY dataPDE DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

message(STATUS "TwoDLookupTable build configured:")
message(STATUS "  Library: ${SOURCES}")
message(STATUS "  Test: test_2d_loader")
message(STATUS "  ROOT: ${ROOT_LIBRARIES}")
