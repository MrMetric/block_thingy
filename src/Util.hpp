#pragma once

#include <cstdint>
#include <string>

namespace Util
{
	std::string read_file(const std::string& path);
	std::string gl_error_string(const uint_fast32_t code);
};
