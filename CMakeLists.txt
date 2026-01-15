cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)
project(Engine)

  set(hotreload true)
  set(X11_use true)
  set(sanitize_address true)
  set(binary_dir "${CMAKE_SOURCE_DIR}/bin")

  message("hotreload: ${hotreload}")
  message("binary path: ${binary_dir}")
  message("sanitizer: ${sanitize_address}")

  # binary output
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${binary_dir}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${binary_dir}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${binary_dir}")
  # .lib output
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${binary_dir}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${binary_dir}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${binary_dir}")
  # .dll output
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${binary_dir}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${binary_dir}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${binary_dir}")

# Compiler
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wuninitialized -fno-exceptions -fno-rtti -Wno-c99-designator)
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  endif()
  message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

# Sanitizer
  if (sanitize_address)
    add_compile_definitions(ASAN_ENABLED=1)
    add_link_options(-fsanitize=address)
  endif()

# Linker
  set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
  add_link_options(-nostdlib++)
  message("Linker path: ${CMAKE_LINKER}")

# OS
  if (UNIX)
    message("OS: linux")
  elseif (WIN32)
    message("OS: windows")
  endif()

# Build
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_DEBUG=1)
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-flto -ffast-math)
  endif()
  message("Build: ${CMAKE_BUILD_TYPE}")

# Binaries
  if (X11_use)
    find_package(X11 REQUIRED)
    add_compile_definitions(GFX_X11=1)
  endif()
  find_package(Vulkan REQUIRED)
  include_directories(src)
  
  # Hotreload
  if (hotreload)
    add_compile_definitions(HOTRELOAD_BUILD=1)

    file(GLOB_RECURSE all_files  src/*.cpp src/*.c)
    file(GLOB_RECURSE game_files src/game/*.cpp)
    set(exe_file src/main.cpp)
    set(lib_files ${all_files})
    list(REMOVE_ITEM lib_files ${game_files} ${exe_file})

    # lib
    add_library(lib SHARED ${lib_files})
    target_compile_definitions(lib PRIVATE KEXPORT)
    target_link_libraries(lib Vulkan::Vulkan)
    if (UNIX)
      if (X11_use)
        target_link_libraries(lib ${X11_LIBRARIES})
        target_link_libraries(lib libxcb-keysyms.so)
      endif()
        target_link_libraries(lib wayland-client.so)
    endif()

    # game
    add_library(game SHARED ${game_files})
    target_include_directories(game PRIVATE src/game)
    target_link_libraries(game ${binary_dir}/liblib.so)

    # exe
    add_executable(exe ${exe_file})
    target_link_libraries(exe ${binary_dir}/liblib.so)

  # One Exe
  else()
    file(GLOB_RECURSE binary_files src/*.cpp src/*.c)
    add_executable(exe ${binary_files})
    target_link_libraries(exe Vulkan::Vulkan)

    if (UNIX)
      if (X11_use)
        target_link_libraries(exe ${X11_LIBRARIES})
        target_link_libraries(exe libxcb-keysyms.so)
      else()
        target_link_libraries(exe wayland-client.so)
      endif()
    elseif (WIN32)
      set_target_properties(exe PROPERTIES WIN32_EXECUTABLE true)
    endif()
  endif()
