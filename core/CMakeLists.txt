cmake_minimum_required(VERSION 3.5.0)

project(Core)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/core/src/*.cpp)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
add_library(core SHARED ${SOURCES})

target_include_directories(core PRIVATE ${CMAKE_SOURCE_DIR}/core/src)

target_compile_definitions(core PRIVATE _DEBUG KEXPORT _CRT_SECURE_NO_WARNINGS)
set_target_properties(core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin #output binaries .lib import
)

