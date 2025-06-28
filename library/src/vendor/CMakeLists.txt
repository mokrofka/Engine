cmake_minimum_required(VERSION 3.5.0)
project(Vendor)

# include
include_directories(${VULKAN_SDK}/Include)

# binary
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/library/src/*.cpp)
add_library(library SHARED ${SOURCES}) # dll

# define
add_compile_definitions(KEXPORT)

# library link
find_package(Vulkan REQUIRED)
target_link_libraries(library Vulkan::Vulkan)

