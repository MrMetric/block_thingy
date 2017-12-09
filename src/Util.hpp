#pragma once

#include <algorithm>
#include <string>

#include <glad/glad.h>

#include "util/filesystem.hpp"

namespace Util {

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

inline std::string strip(std::string s, const std::string& chars)
{
	s.erase(0, s.find_first_not_of(chars));
	s.erase(s.find_last_not_of(chars) + 1);
	return s;
}

inline std::string strip_whitespace(std::string s)
{
	return strip(s, " \t\r\n");
}

template<typename T>
inline void delete_element(T& t, typename T::value_type e)
{
	auto i = std::remove(t.begin(), t.end(), e);
	if(i != t.end())
	{
		t.erase(i, t.end());
	}
}

bool file_is_openable(const fs::path&);
std::string read_text(const fs::path&);
std::string read_file(const fs::path&);
std::string read_file(const fs::path&, bool is_text);

std::string gl_object_log(GLuint object);

void change_directory(const fs::path&);

int stoi(const std::string&);

std::string datetime();

}
