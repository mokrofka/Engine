cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)
project(BuildEverything)

# Options
# NOTE you need manually set it up
option(build_one_exe "" false)  # cmake -B build -S . -Dbuild_one_exe=true
option(sanitize_address "" OFF) # cmake -B build -S . -Dsanitize_address=true

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

# Compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
  add_compile_options(-Wuninitialized -fno-exceptions -fno-rtti -Wno-c99-designator)
endif()

# Linker
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
message("Linker path: ${CMAKE_LINKER}")

# OS
if (WIN32)
  message("OS: windows")
elseif (UNIX)
  message("OS: linux")
endif()

# Build
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  message("Build: Release")
  add_compile_options(-flto -ffast-math)
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("Build: Debug")
  add_compile_definitions(BUILD_DEBUG=1 ASAN_ENABLED=1)
endif()

# Sanitizer
if (sanitize_address)
  message("sanitizer is ON")
  add_link_options(-fsanitize=address)
else()
  message("sanitizer is OFF")
endif()

# Build one or many binaries
if (build_one_exe)
  message("one exe: true")
  add_subdirectory(app)
else()
  message("one exe: false")
  add_subdirectory(library)
  add_subdirectory(game)
  add_subdirectory(app)
endif()
