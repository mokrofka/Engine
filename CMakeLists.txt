cmake_minimum_required(VERSION 3.5.0)

project(BuildEverything)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wuninitialized -Wvarargs -fno-exceptions -fno-rtti -Wno-writable-strings -Wno-microsoft-goto -Wno-sizeof-array-div)

# binary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)

# .lib export
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(app)
