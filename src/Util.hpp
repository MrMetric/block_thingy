#pragma once

#include <algorithm>
#include <string>

#include <glad/glad.h>

namespace Util
{
	// http://stackoverflow.com/a/2072890/1578318
	inline bool string_ends_with(const std::string& value, const std::string& ending)
	{
		if(ending.size() > value.size())
		{
			return false;
		}
		return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
	}

	bool file_is_openable(const std::string& path);
	std::string read_file(const std::string& path);

	std::string gl_error_string(GLenum code);
	std::string gl_object_log(GLuint object);

	struct path
	{
		std::string folder;
		std::string file;
		std::string ext;
	};

	path split_path(const std::string&);
	std::string join_path(const path&);

	void change_directory(const std::string&);

	template <typename T> constexpr T clamp(T x, T min, T max)
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

	double mod(double x, double y);
};
