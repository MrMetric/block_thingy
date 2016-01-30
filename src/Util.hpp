#pragma once

#include <cstdint>
#include <string>

class Util
{
	public:
		static std::string read_file(const std::string& path);
		static std::string gl_error_string(const uint_fast32_t code);
};