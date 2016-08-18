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

set_property(TARGET block_thingy PROPERTY CXX_STANDARD 14)
set_property(TARGET block_thingy PROPERTY CXX_STANDARD_REQUIRED ON)
target_compile_features(block_thingy PRIVATE cxx_relaxed_constexpr)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

target_compile_options(block_thingy PRIVATE
	$<$<CONFIG:Debug>:-fsanitize=undefined,integer>
	$<$<CONFIG:RelWithDebInfo>:-fsanitize=undefined,integer>
	-march=native
	-DUSE_LIBPNG
	-DMSGPACK_DISABLE_LEGACY_NIL
	-DMSGPACK_DISABLE_LEGACY_CONVERT
	${GLFW_CFLAGS}
)
target_link_libraries(block_thingy
	$<$<CONFIG:Debug>:-fsanitize=undefined,integer>
	$<$<CONFIG:RelWithDebInfo>:-fsanitize=undefined,integer>
	${GLFW_LDFLAGS}
	-lpng
	-lPocoFoundation
)
