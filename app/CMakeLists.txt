cmake_minimum_required(VERSION 3.5.0)

project(App)

include_directories(${CMAKE_SOURCE_DIR}/core/src)
include_directories(${CMAKE_SOURCE_DIR}/engine/src)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/app/src/*.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)

add_executable(app ${SOURCES})
target_compile_definitions(app PRIVATE _DEBUG _CRT_SECURE_NO_WARNINGS)

target_link_libraries(app "${CMAKE_SOURCE_DIR}/bin/core.lib")
target_link_libraries(app "${CMAKE_SOURCE_DIR}/bin/engine.lib")
