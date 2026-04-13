cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 23)
project(Engine)

# options
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
  add_compile_options(-Wuninitialized -fno-exceptions -fno-rtti -Wno-c99-designator)
  message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

# Sanitizer
  if (sanitize_address)
    add_compile_definitions(ASAN_ENABLED=1)
    add_link_options(-fsanitize=address)
  endif()

# Linker
  add_link_options(-nostdlib++ "-fuse-ld=lld")
  message("Linker path: ${CMAKE_LINKER}")

# Build
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_DEBUG=1)
  # elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message("hello")
    add_compile_options(-flto -ffast-math)
  endif()
  message("Build: ${CMAKE_BUILD_TYPE}")

  if (X11_use)
    find_package(X11 REQUIRED)
    add_compile_definitions(GFX_X11=1)
  endif()
  include_directories(src)
  include_directories(vendor)
  
# Hotreload
  if (hotreload)
    add_compile_definitions(HOTRELOAD_BUILD=1)

    # Lib
    add_library(lib SHARED 
      src/base/base_impl.cpp
      src/os/os_impl.cpp
      src/vk.cpp
      src/stb_impl.c
      src/common.cpp
    )
    target_compile_definitions(lib PRIVATE KEXPORT)
    if (UNIX)
      if (X11_use)
        target_link_libraries(lib ${X11_LIBRARIES} libxcb-keysyms.so -lxcb)
      endif()
        target_link_libraries(lib wayland-client.so)
    endif()

    # Game
    add_library(game SHARED src/game/game.cpp)
    target_include_directories(game PRIVATE src/game)
    target_link_libraries(game ${binary_dir}/liblib.so)

    # Exe
    add_executable(exe src/main.cpp)
    target_link_libraries(exe ${binary_dir}/liblib.so)

# One Exe
  else()
    add_executable(exe
      src/base/base_impl.cpp
      src/os/os_impl.cpp
      src/vk.cpp
      src/stb_impl.c
      src/common.cpp
      src/game/game.cpp
      src/main.cpp
    )
    if (UNIX)
      if (X11_use)
        target_link_libraries(exe ${X11_LIBRARIES} libxcb-keysyms.so -lxcb)
      else()
        target_link_libraries(exe wayland-client.so)
      endif()
    elseif (WIN32)
      set_target_properties(exe PROPERTIES WIN32_EXECUTABLE true)
    endif()
  endif()

# add_executable(compiling_shaders 
#   src/base/base_impl.cpp
#   src/os/os_impl.cpp
#   src/compiling_shaders.cpp
# )
# target_link_libraries(compiling_shaders ${X11_LIBRARIES} libxcb-keysyms.so -lxcb)
