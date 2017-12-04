#pragma once

// check for <experimental/filesystem> first because MSVC errors with <filesystem>
#if __has_include(<experimental/filesystem>)
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#elif __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#error The C++ filesystem library is missing
#endif
