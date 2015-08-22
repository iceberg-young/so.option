cmake_minimum_required(VERSION 3.2)

project(option)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES example.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

add_subdirectory(source)
add_dependencies(${PROJECT_NAME} so.option)
target_link_libraries(${PROJECT_NAME} so.option)

include(Dependencies.cmake)
