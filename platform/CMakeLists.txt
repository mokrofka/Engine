# CMakeLists.txt for the testbed project

cmake_minimum_required(VERSION 3.5.0)

project(Platform)

include_directories(${CMAKE_SOURCE_DIR}/core/src)
include_directories(${CMAKE_SOURCE_DIR}/engine/src)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/platform/src/*.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)

add_executable(platform ${SOURCES})
target_compile_definitions(platform PRIVATE _DEBUG)

target_link_libraries(platform "${CMAKE_SOURCE_DIR}/bin/core.lib")
target_link_libraries(platform "${CMAKE_SOURCE_DIR}/bin/engine.lib")

# target_compile_options(core PRIVATE -Wuninitialized -Wvarargs -fno-exceptions -fno-rtti)

# add_custom_command(
#     TARGET platform
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E env "cmd.exe" "/C" "${CMAKE_SOURCE_DIR}/post-build.bat"
# )
