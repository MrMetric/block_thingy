#pragma once

#include <algorithm>
#include <string>

#include <glad/glad.h>

#include "util/filesystem.hpp"

namespace Util
{
	inline bool string_starts_with(const std::string& value, const std::string& start)
	{
		if(start.size() > value.size())
		{
			return false;
		}
		return std::equal(start.cbegin(), start.cend(), value.cbegin());
	}

	// http://stackoverflow.com/a/2072890/1578318
	inline bool string_ends_with(const std::string& value, const std::string& ending)
	{
		if(ending.size() > value.size())
		{
			return false;
		}
		return std::equal(ending.crbegin(), ending.crend(), value.crbegin());
	}

	bool file_is_openable(const fs::path&);
	std::string read_file(const fs::path&);

	std::string gl_object_log(GLuint object);

	void change_directory(const fs::path&);

	template<typename T>
	constexpr T clamp(T x, T min, T max)
	{
		if(x < min)
		{
			return min;
		}
		if(x > max)
		{
			return max;
		}
		return x;
	}

	int stoi(const std::string&);

	std::string datetime();
}
