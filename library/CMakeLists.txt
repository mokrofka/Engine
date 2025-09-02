cmake_minimum_required(VERSION 3.10.0)
project(Library)

# include
include_directories(${CMAKE_SOURCE_DIR}/library/src)
include_directories(${VULKAN_SDK}/Include)

# binary
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/library/src/*.cpp ${CMAKE_SOURCE_DIR}/library/src/*.c)
add_library(library SHARED ${SOURCES}) # dll

# define
add_compile_definitions(KEXPORT)

# library link
find_package(Vulkan REQUIRED)
target_link_libraries(library Vulkan::Vulkan)

if (UNIX)
  target_link_libraries(library /usr/lib/libwayland-client.so)
endif()

