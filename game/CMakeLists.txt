cmake_minimum_required(VERSION 3.5.0)

project(Game)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/game/src/*.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
add_library(game SHARED ${SOURCES})

target_include_directories(game PRIVATE ${CMAKE_SOURCE_DIR}/engine/src)
target_include_directories(game PRIVATE ${CMAKE_SOURCE_DIR}/core/src)

target_link_libraries(game "${CMAKE_SOURCE_DIR}/bin/core.lib")
target_link_libraries(game "${CMAKE_SOURCE_DIR}/bin/engine.lib")

set_target_properties(game PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin #output binaries .lib import
)

target_compile_definitions(game PRIVATE _DEBUG _CRT_SECURE_NO_WARNINGS)
# target_compile_options(game PRIVATE -Wuninitialized -Wvarargs -fno-exceptions -fno-rtti)
