cmake_minimum_required(VERSION 3.5.0)
project(Exe)

# include
include_directories(${CMAKE_SOURCE_DIR}/app/src)
include_directories(${CMAKE_SOURCE_DIR}/engine/src)

# binary
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/app/src/*.cpp)
add_executable(app ${SOURCES}) # exe

# define
target_compile_definitions(app PRIVATE _DEBUG)

# library link
target_link_libraries(app "${CMAKE_SOURCE_DIR}/bin/engine.lib")

# link properties
# set_target_properties(app PROPERTIES
#   WIN32_EXECUTABLE ON
# )
