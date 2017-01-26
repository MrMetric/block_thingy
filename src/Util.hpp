#pragma once

#include <algorithm>
#include <string>

#include <glad/glad.h>

// http://www.lighthouse3d.com/cg-topics/error-tracking-in-opengl/
// http://www.lighthouse3d.com/cg-topics/error-tracking-in-opengl/
void printOglError(const std::string& file, const int line, const std::string& func);
#define printOpenGLError() printOglError(__FILE__, __LINE__, __func__)

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

	bool file_is_openable(const std::string&);
	std::string read_file(const std::string&);

	std::string gl_error_string(GLenum code);
	std::string gl_object_log(GLuint object);

	struct path
	{
		std::string folder;
		std::string file;
		std::string ext;
	};
	path split_path(std::string);
	std::string join_path(const path&);

	void change_directory(const std::string&);
	bool create_directory(const std::string&);
	bool create_directories(const std::string&);

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
};
