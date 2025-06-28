cmake_minimum_required(VERSION 3.5.0)
project(BuildEverything)

option(build_one_exe "" false) # NOTE you need manually set it up
# cmake -B build -S . -Dbuild_one_exe=true

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
  message("one exe: true")
else ()
  message("one exe: false")
  add_subdirectory(library)
  add_subdirectory(game)
  add_subdirectory(app)
endif()

set_target_properties(app PROPERTIES
  WIN32_EXECUTABLE false
)
