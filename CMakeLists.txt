set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
cmake_minimum_required(VERSION 4.4.2)
set(CMAKE_CXX_STANDARD 23)
project(Engine)

# options
set(binary_dir "${CMAKE_SOURCE_DIR}/bin")
set(hotreload true)
set(X11_use true)
set(Wayland_use false)
set(sanitize_address true)
set(dev_build true)

message("binary path: ${binary_dir}")
message("hotreload: ${hotreload}")
message("X11_use: ${X11_use}")
message("sanitizer: ${sanitize_address}")
message("dev build: ${dev_build}")
message("Compiler: ${CMAKE_CXX_COMPILER_ID}")
message("Linker path: ${CMAKE_LINKER}")
message("Build: ${CMAKE_BUILD_TYPE}")

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
add_compile_options(-Wunused-variable -Wuninitialized -Wno-unused-const-variable -Wno-gnu-alignof-expression -Wno-c99-designator -fno-exceptions -fno-rtti -mavx)

# Sanitizer
if(sanitize_address)
	add_compile_definitions(ASAN_ENABLED=1)
	add_link_options(-fsanitize=address)
endif()

# Linker
add_link_options("-fuse-ld=lld" "-rdynamic")

# Build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(BUILD_DEBUG=1)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
	add_compile_options(-flto -ffast-math)
endif()

if(X11_use)
	add_compile_definitions(GFX_X11=1)
elseif(Wayland_Use)
	add_compile_definitions(GFX_WAYLAND=1)
endif()
include_directories(src)
include_directories(src/base)
include_directories(vendor)

set(game_source src/com.cpp)
set(exe_source src/main.cpp vendor/vendor_impl.cpp)
set(X11_libs -lxcb -lxcb-xkb -lxcb-keysyms -lxcb-xinput)

if(dev_build)
	add_compile_definitions(BUILD_DEV=1)
	set_source_files_properties(
		vendor/vendor_impl.cpp
		PROPERTIES
		COMPILE_OPTIONS "-O0;-g"
	)
else()
	add_link_options("-nostdlib++") # imgui depends on it
endif()

add_executable(exe)

# Hotreload
if(hotreload)
	add_compile_definitions(HOTRELOAD_BUILD=1)

	# Game
	add_library(game SHARED ${game_source})

	# Exe
	target_sources(exe PRIVATE ${exe_source})
	if(UNIX)
		if(X11_use)
			target_link_libraries(exe ${X11_libs})
		endif()
			target_link_libraries(exe wayland-client.so)
	endif()

# One Exe
else()
	target_sources(exe PRIVATE
		${game_source}
		${exe_source}
	)
	if(UNIX)
		if(X11_use)
			target_link_libraries(exe ${X11_libs})
		else()
			target_link_libraries(exe wayland-client.so)
		endif()
	elseif (WIN32)
		set_target_properties(exe PROPERTIES WIN32_EXECUTABLE true)
	endif()
endif()

