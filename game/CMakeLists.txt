cmake_minimum_required(VERSION 3.5.0)
project(Game)

# include
include_directories(${CMAKE_SOURCE_DIR}/game/src)
include_directories(${CMAKE_SOURCE_DIR}/engine/src)

# binary
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/game/src/*.cpp)
add_library(game SHARED ${SOURCES}) #dll

# define
target_compile_definitions(game PRIVATE _DEBUG)

# library link
target_link_libraries(game "${CMAKE_SOURCE_DIR}/bin/engine.lib")
