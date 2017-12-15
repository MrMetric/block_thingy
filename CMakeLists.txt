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

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
	get_filename_component(MAC_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
	set(CPP_FS_LIB -lc++experimental "-L${MAC_BIN_DIR}/../lib")
else()
	option(USE_LIBCPP "Use libc++ instead of libstdc++ (requires Clang)" FALSE)
	if(${USE_LIBCPP})
		if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
			message(FATAL_ERROR "Using libc++ requires building with Clang")
		endif()
		message(STATUS "Using libc++")
		set(CPP_FS_LIB -lc++experimental)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	else()
		set(CPP_FS_LIB -lstdc++fs)
	endif()
endif()

set_property(TARGET block_thingy PROPERTY CXX_STANDARD 17)
set_property(TARGET block_thingy PROPERTY CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_search_module(FREETYPE REQUIRED freetype2)
pkg_search_module(GLFW REQUIRED glfw3>=3.2)
pkg_search_module(LIBPNG REQUIRED libpng)
pkg_search_module(MSGPACK REQUIRED msgpack>=2.0.0) # TODO: determine minimum version

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(FSANITIZE -fsanitize=undefined,thread)
	set(FLAGS
		-isystem "${PROJECT_SOURCE_DIR}/lib"
		-iquote "${PROJECT_SOURCE_DIR}/src"
		-Weverything
		-Werror=cast-qual
		-Werror=conversion
		-Werror=delete-incomplete
		-Werror=deprecated
		-Werror=extra-semi
		-Werror=extra-tokens
		-Werror=float-conversion
		-Werror=inconsistent-missing-override
		-Werror=invalid-pp-token
		-Werror=mismatched-tags
		-Werror=missing-declarations
		-Werror=missing-field-initializers
		-Werror=newline-eof
		-Werror=non-virtual-dtor
		-Werror=old-style-cast
		-Werror=return-type
		-Werror=self-move
		-Werror=shorten-64-to-32
		-Werror=sign-compare
		-Werror=sign-conversion
		-Werror=strict-prototypes
		-Werror=string-plus-int
		-Werror=uninitialized
		-Werror=unknown-pragmas
		-Werror=unknown-warning-option
		-Werror=unused-result
		-Werror=weak-vtables
		-Werror=zero-as-null-pointer-constant
		-Wno-c++98-compat-pedantic
		-Wno-documentation-unknown-command
		-Wno-exit-time-destructors
		-Wno-float-equal
		-Wno-global-constructors
		-Wno-logical-op-parentheses
		-Wno-missing-braces
		-Wno-padded
		-Wno-shadow
		-Wno-undefined-func-template
	)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(FSANITIZE -fsanitize=undefined,thread)
	set(FLAGS
		-isystem "${PROJECT_SOURCE_DIR}/lib"
		-iquote "${PROJECT_SOURCE_DIR}/src"
		-pedantic
		-Wall
		-Wextra
		-Wnull-dereference
		-Werror=missing-field-initializers
		-Werror=old-style-cast
		-Werror=return-type
		-Wno-parentheses
	)
else()
	include_directories(
		"lib"
		"src"
	)
endif()

target_compile_options(block_thingy PRIVATE
	$<${DEBUG_BUILD}:${FSANITIZE}>
	-march=native
	-fno-math-errno
	-fno-signed-zeros
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
