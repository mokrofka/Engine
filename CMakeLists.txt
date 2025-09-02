cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)
project(BuildEverything)

# binary output
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)
# .lib output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)
# .dll output
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)

option(build_one_exe "" false) # NOTE you need manually set it up
# cmake -B build -S . -Dbuild_one_exe=true

add_compile_options(-Wuninitialized -fno-exceptions -fno-rtti -Wno-c99-designator)

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

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message("Release")
  add_compile_options(-flto -ffast-math)
endif()
if (WIN32)
  message("It's windows")
elseif (UNIX)
  message("It's linux")
endif()
