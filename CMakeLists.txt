cmake_minimum_required(VERSION 3.1.0)
project(block_thingy)

message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")

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
	"lib/rhea/*.cpp"
	"src/*.cpp"
)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	list(APPEND block_thingy_SRC "lib/inotify-cxx/inotify-cxx.cpp")
	add_definitions(
		-DHAVE_POSIX
		-DUSE_INOTIFY
	)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
	add_definitions(
		-DHAVE_POSIX
	)
endif()
add_executable(block_thingy ${block_thingy_SRC})

option(USE_LIBCPP "Use libc++ instead of libstdc++ (requires Clang)" FALSE)
if(${USE_LIBCPP})
	if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		message(FATAL_ERROR "Using libc++ requires building with Clang")
	endif()
	message(STATUS "Using libc++")
	set(CPP_FS_LIB -lc++experimental)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else()
	# TODO: libc++ is the default on Mac OS X, so this will not work
	set(CPP_FS_LIB -lstdc++fs)
endif()

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

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(FSANITIZE -fsanitize=undefined,thread)
	set(FLAGS
		-isystem "${PROJECT_SOURCE_DIR}/lib"
		-iquote "${PROJECT_SOURCE_DIR}/src"
		-Weverything
		-Werror=delete-incomplete
		-Werror=deprecated
		-Werror=extra-tokens
		-Werror=inconsistent-missing-override
		-Werror=invalid-pp-token
		-Werror=missing-field-initializers
		-Werror=return-type
		-Werror=sign-compare
		-Werror=uninitialized
		-Werror=unknown-pragmas
		-Werror=unknown-warning-option
		-Wno-c++98-compat-pedantic
		-Wno-deprecated-dynamic-exception-spec
		-Wno-documentation-unknown-command
		-Wno-exit-time-destructors
		-Wno-float-equal
		-Wno-global-constructors
		-Wno-padded
		-Wno-shadow
		-Wno-undefined-func-template
	)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(FLAGS
		-isystem "${PROJECT_SOURCE_DIR}/lib"
		-iquote "${PROJECT_SOURCE_DIR}/src"
		-pedantic
		-Wall
		-Wextra
		-Werror=return-type
	)
else()
	include_directories(
		"lib"
		"src"
	)
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
	${FLAGS}
)


target_link_libraries(block_thingy
	$<${DEBUG_BUILD}:${FSANITIZE}>
	-flto
	${CPP_FS_LIB}
	${FREETYPE_LDFLAGS}
	${GLFW_LDFLAGS}
	${LIBPNG_LDFLAGS}
	${MSGPACK_LDFLAGS}
	-ldl
	-lpthread
	-lz
)
