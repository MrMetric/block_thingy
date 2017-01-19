cmake_minimum_required(VERSION 3.1.0)
project(block_thingy)

file(GLOB_RECURSE block_thingy_SRC
	"lib/glad/glad.c"
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
pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(FREETYPE REQUIRED freetype2)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(FSANITIZE -fsanitize=undefined,integer)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
target_compile_options(block_thingy PRIVATE
	$<$<CONFIG:Debug>:${FSANITIZE}>
	$<$<CONFIG:RelWithDebInfo>:${FSANITIZE}>
	-march=native
	-DUSE_LIBPNG
	-DMSGPACK_DISABLE_LEGACY_NIL
	-DMSGPACK_DISABLE_LEGACY_CONVERT
	-DGLFW_INCLUDE_NONE
	-DGLM_FORCE_SIZE_T_LENGTH
	-DGLM_FORCE_SIZE_FUNC
	-DGLM_FORCE_EXPLICIT_CTOR
	-DGLM_ENABLE_EXPERIMENTAL
	${GLFW_CFLAGS}
	${FREETYPE_CFLAGS}
)
target_link_libraries(block_thingy
	$<$<CONFIG:Debug>:${FSANITIZE}>
	$<$<CONFIG:RelWithDebInfo>:${FSANITIZE}>
	${GLFW_LDFLAGS}
	${FREETYPE_LDFLAGS}
	-lpng
	-lz
)
