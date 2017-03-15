cmake_minimum_required(VERSION 3.1.0)
project(block_thingy)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
	set(DEBUG_BUILD 1)
else()
	set(DEBUG_BUILD 0)
endif()
file(GLOB_RECURSE block_thingy_SRC
	"lib/easylogging++/easylogging++.cpp"
	"lib/glad/glad.c"
	"lib/inotify-cxx/inotify-cxx.cpp"
	"src/*.cpp"
)
include_directories(
	"lib"
	"src"
)
add_executable(block_thingy ${block_thingy_SRC})

#[[
set_property(TARGET block_thingy PROPERTY CXX_STANDARD 14)
set_property(TARGET block_thingy PROPERTY CXX_STANDARD_REQUIRED ON)
target_compile_features(block_thingy PRIVATE cxx_relaxed_constexpr)
]]

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3>=3.2)
pkg_search_module(FREETYPE REQUIRED freetype2)
pkg_search_module(LIBPNG REQUIRED libpng)
pkg_search_module(MSGPACK REQUIRED msgpack>=2.0.0) # TODO: determine minimum version

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(FSANITIZE -fsanitize=undefined,integer)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
target_compile_options(block_thingy PRIVATE
	$<${DEBUG_BUILD}:${FSANITIZE}>
	-march=native
	$<${DEBUG_BUILD}:-DDEBUG_BUILD>
	-DGLFW_INCLUDE_NONE
	-DGLM_ENABLE_EXPERIMENTAL
	-DGLM_FORCE_EXPLICIT_CTOR
	-DGLM_FORCE_SIZE_FUNC
	-DGLM_FORCE_SIZE_T_LENGTH
	-DMSGPACK_DISABLE_LEGACY_CONVERT
	-DMSGPACK_DISABLE_LEGACY_NIL
	${FREETYPE_CFLAGS}
	${GLFW_CFLAGS}
	${LIBPNG_CFLAGS}
	${MSGPACK_CFLAGS}
)

target_link_libraries(block_thingy
	$<${DEBUG_BUILD}:${FSANITIZE}>
	-flto
	-lstdc++fs
	${FREETYPE_LDFLAGS}
	${GLFW_LDFLAGS}
	${LIBPNG_LDFLAGS}
	${MSGPACK_LDFLAGS}
	-lz
)
