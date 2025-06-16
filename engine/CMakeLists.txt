cmake_minimum_required(VERSION 3.5.0)

project(Engine)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/engine/src/*.cpp)
add_library(engine SHARED ${SOURCES})

target_include_directories(engine PRIVATE ${CMAKE_SOURCE_DIR}/engine/src)
target_include_directories(engine PRIVATE ${CMAKE_SOURCE_DIR}/core/src/containers)
target_include_directories(engine PRIVATE ${CMAKE_SOURCE_DIR}/core/src)
target_include_directories(engine PRIVATE ${VULKAN_SDK}/Include)

find_package(Vulkan REQUIRED)
target_link_libraries(engine Vulkan::Vulkan)

target_compile_definitions(engine PRIVATE _DEBUG KEXPORT _CRT_SECURE_NO_WARNINGS)

