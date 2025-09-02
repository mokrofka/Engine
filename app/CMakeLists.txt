cmake_minimum_required(VERSION 3.10.0)
project(Exe)

# include
include_directories(${CMAKE_SOURCE_DIR}/app/src)
include_directories(${CMAKE_SOURCE_DIR}/library/src)
include_directories(${CMAKE_SOURCE_DIR}/game/src)

# binary
if (build_one_exe)
  file(GLOB_RECURSE ExeSrc ${CMAKE_SOURCE_DIR}/app/src/*.cpp)
  file(GLOB_RECURSE LibrarySrc ${CMAKE_SOURCE_DIR}/library/src/*.cpp ${CMAKE_SOURCE_DIR}/library/src/*.c)
  file(GLOB_RECURSE GameSrc ${CMAKE_SOURCE_DIR}/game/src/*.cpp)

  set(OneExe
    ${ExeSrc}
    ${LibrarySrc}
    ${GameSrc}
  )

  add_executable(app ${OneExe})

  # boil

  find_package(Vulkan REQUIRED)
  target_link_libraries(app Vulkan::Vulkan)
  if (WIN32)
    set_target_properties(app PROPERTIES WIN32_EXECUTABLE true)
  elseif (UNIX)
    target_link_libraries(app /usr/lib/libwayland-client.so)
  endif()
  
  add_compile_definitions(MONOLITHIC_BUILD)
else ()
  file(GLOB_RECURSE ExeSrc ${CMAKE_SOURCE_DIR}/app/src/*.cpp)
  add_executable(app ${ExeSrc})
  target_link_libraries(app "${CMAKE_SOURCE_DIR}/bin/liblibrary.so")
endif()
