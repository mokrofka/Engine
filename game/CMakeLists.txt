cmake_minimum_required(VERSION 3.5.0)
project(Game)

# include
include_directories(${CMAKE_SOURCE_DIR}/game/src)
include_directories(${CMAKE_SOURCE_DIR}/library/src)

# binary
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/game/src/*.cpp)
add_library(game SHARED ${SOURCES}) # dll

# define
add_compile_definitions()

# library link
target_link_libraries(game "${CMAKE_SOURCE_DIR}/bin/library.lib")
