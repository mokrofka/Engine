cmake_minimum_required(VERSION 3.5.0)
project(Library)

# include
include_directories(${CMAKE_SOURCE_DIR}/engine/src)
include_directories(${VULKAN_SDK}/Include)

# binary
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/engine/src/*.cpp)
add_library(engine SHARED ${SOURCES}) # dll

# define
target_compile_definitions(engine PRIVATE _DEBUG KEXPORT)

# library link
find_package(Vulkan REQUIRED)
target_link_libraries(engine Vulkan::Vulkan)

