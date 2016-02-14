cmake_minimum_required(VERSION 3.1.0)
project(block_thingy)

file(GLOB_RECURSE block_thingy_SRC
	"src/*.cpp"
	"lib/glad/glad.c"
)
include_directories("lib")
add_executable(block_thingy ${block_thingy_SRC})

set_property(TARGET block_thingy PROPERTY CXX_STANDARD 14)
set_property(TARGET block_thingy PROPERTY CXX_STANDARD_REQUIRED ON)
target_compile_features(block_thingy PRIVATE cxx_relaxed_constexpr)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
target_compile_options(block_thingy PRIVATE -march=native -O2)
target_link_libraries(block_thingy ${GLFW_STATIC_LIBRARIES})