cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 23)
project(Engine)

# options
  set(hotreload true)
  set(X11_use true)
  set(sanitize_address true)
  set(binary_dir "${CMAKE_SOURCE_DIR}/bin")
  set(dear_imgui true)
  set(profile true)

  message("hotreload: ${hotreload}")
  message("binary path: ${binary_dir}")
  message("sanitizer: ${sanitize_address}")
  message("dear imgui: ${dear_imgui}")

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
  add_compile_options(-Wunused-variable -Wno-unused-const-variable -Wuninitialized -fno-exceptions -fno-rtti -Wno-c99-designator)
  message("Compiler: ${CMAKE_CXX_COMPILER_ID}")

# Sanitizer
  if (sanitize_address)
    add_compile_definitions(ASAN_ENABLED=1)
    add_link_options(-fsanitize=address)
  endif()

# Linker
  add_link_options("-fuse-ld=lld")
  message("Linker path: ${CMAKE_LINKER}")

# Build
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BUILD_DEBUG=1)
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release" OR
          CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_options(-flto -ffast-math)
  endif()
  message("Build: ${CMAKE_BUILD_TYPE}")

  if (X11_use)
    add_compile_definitions(GFX_X11=1)
  endif()
  if (profile)
    add_compile_definitions(PROFILE_BUILD)
  endif()
  include_directories(src)
  include_directories(vendor)
  
  set(lib_source
    src/base/base_impl.cpp
    vendor/stb_impl.c
  )
  set(game_source src/common.cpp)
  set(exe_source src/main.cpp)

  if (dear_imgui)
    list(APPEND lib_source vendor/imgui/imgui_impl.cpp)
    set_source_files_properties(
      vendor/imgui/imgui_impl.cpp
      vendor/stb_impl.c
      PROPERTIES
      COMPILE_OPTIONS "-O0;-g"
    )
    add_compile_definitions(DEAR_IMGUI=1)
  else()
    add_link_options("-nostdlib++")
  endif()

# Hotreload
  if (hotreload)
    add_compile_definitions(HOTRELOAD_BUILD=1)

    # Lib
    add_library(lib SHARED ${lib_source})
    target_compile_definitions(lib PRIVATE KEXPORT)
    if (UNIX)
      if (X11_use)
        target_link_libraries(lib -lxcb libxcb-keysyms.so )
      endif()
        target_link_libraries(lib wayland-client.so)
    endif()

    # Game
    add_library(game SHARED ${game_source})
    target_include_directories(game PRIVATE src/game)
    target_link_libraries(game ${binary_dir}/liblib.so)

    # Exe
    add_executable(exe ${exe_source})
    target_link_libraries(exe ${binary_dir}/liblib.so)

# One Exe
  else()
    add_executable(exe
      ${lib_source}
      ${game_source}
      ${exe_source}
    )
    if (UNIX)
      if (X11_use)
        target_link_libraries(exe -lxcb libxcb-keysyms.so)
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
# target_link_libraries(compiling_shaders -lxcb libxcb-keysyms.so)
