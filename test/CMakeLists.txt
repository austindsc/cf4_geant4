cmake_minimum_required(VERSION 3.7)
project(cversion)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GSL REQUIRED)

# set(CMAKE_CXX_FLAGS "-std=c++14 -lstdc++fs" )
set(SOURCE_FILES cversion.cpp)

add_executable(cversion ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} GSL::gsl)
