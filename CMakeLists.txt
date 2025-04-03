cmake_minimum_required(VERSION 3.5.0)

project(BuildEverything)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wuninitialized -Wvarargs -fno-exceptions -fno-rtti)

add_subdirectory(core)
add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(app)
