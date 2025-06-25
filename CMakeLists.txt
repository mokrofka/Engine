cmake_minimum_required(VERSION 3.5.0)
project(BuildEverything)

option(build_one_exe "" true)

add_compile_options(-Wuninitialized -fno-exceptions -fno-rtti -Wno-writable-strings )

# binary output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)

# .lib output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)

# build
if (build_one_exe)
  add_subdirectory(app)
  message("yea")
else ()
  message("false")
  add_subdirectory(engine)
  add_subdirectory(game)
  add_subdirectory(app)
endif()

# cmake -B build -S . -Dbuild_one_exe=true
