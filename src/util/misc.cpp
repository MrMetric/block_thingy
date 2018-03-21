#include "misc.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>

#include <glad/glad.h>

#include "util/logger.hpp"

using std::nullopt;
using std::string;
using std::unique_ptr;

namespace block_thingy::util {

bool file_is_openable(const fs::path& path)
{
	return std::ifstream(path).is_open();
}

string read_text(const fs::path& path)
{
	return read_file(path, true);
}

string read_file(const fs::path& path)
{
	return read_file(path, false);
}

string read_file(const fs::path& path, const bool is_text)
{
	if(fs::is_directory(path))
	{
		throw std::runtime_error("failed to read " + path.u8string() + " because it is a directory");
	}
	try
	{
		std::ifstream inpoot(path, std::ifstream::ate | std::ifstream::binary);
		inpoot.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		const std::size_t fsize = static_cast<std::size_t>(inpoot.tellg());
		if(fsize == 0)
		{
			return "";
		}
		inpoot.seekg(0);
		unique_ptr<char[]> buf = std::make_unique<char[]>(fsize);
		inpoot.read(buf.get(), static_cast<std::streamsize>(fsize));
		string s(buf.get(), fsize);
		if(is_text && s.find('\r') != string::npos)
		{
			s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
		}
		return s;
	}
	catch(const std::ios_base::failure&)
	{
		LOG(ERROR) << "failed to read " << path.u8string() << '\n';
		throw;
	}
}

string gl_object_log(const GLuint object)
{
	GLint log_length_;
	if(glIsShader(object))
	{
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length_);
	}
	else if(glIsProgram(object))
	{
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length_);
	}
	else
	{
		throw std::runtime_error("can not get log for an OpenGL object that is not a shader or a program");
	}

	if(log_length_ == 0)
	{
		return "";
	}
	assert(log_length_ >= 0);
	const auto log_length = static_cast<std::size_t>(log_length_);

	unique_ptr<char[]> log = std::make_unique<char[]>(log_length);
	if(glIsShader(object))
	{
		glGetShaderInfoLog(object, log_length_, nullptr, log.get());
	}
	else if(glIsProgram(object))
	{
		glGetProgramInfoLog(object, log_length_, nullptr, log.get());
	}
	return string(log.get(), log_length);
}

bool is_integer(const string& s) noexcept
{
	if(s.empty())
	{
		return false;
	}

	string::size_type start_pos;
	if(s[0] == '+' || s[0] == '-')
	{
		if(s.size() == 1)
		{
			return false;
		}
		start_pos = 1;
	}
	else
	{
		start_pos = 0;
	}

	return s.find_first_not_of("0123456789", start_pos) == string::npos;
}

std::optional<unsigned int> stou(const string& s) noexcept
{
	// there is no std::stou
	const std::optional<unsigned long> u = util::stoul(s);
	if(u == nullopt
	|| *u > std::numeric_limits<unsigned int>::max())
	{
		return {};
	}
	return static_cast<unsigned int>(*u);
}

#define STOI(TYPE, NAME) \
std::optional<TYPE> NAME(const string& s) noexcept \
{ \
	if(!is_integer(s)) \
	{ \
		return {}; \
	} \
	try \
	{ \
		return std::NAME(s); \
	} \
	catch(...) \
	{ \
	} \
	return {}; \
}
STOI(unsigned long     , stoul )
STOI(unsigned long long, stoull)
STOI(int               , stoi  )
STOI(long              , stol  )
STOI(long long         , stoll )
#undef STOI

#define STOF(TYPE, NAME) \
std::optional<TYPE> NAME(const string& s) noexcept \
{ \
	try \
	{ \
		const TYPE f = std::NAME(s); \
		if(!std::isnan(f) && !std::isinf(f)) \
		{ \
			return f; \
		} \
	} \
	catch(...) \
	{ \
	} \
	return {}; \
}
STOF(float      , stof )
STOF(double     , stod )
STOF(long double, stold)
#undef STOF

string datetime()
{
	const auto now = std::chrono::system_clock::now();
	const auto us = std::chrono::time_point_cast<std::chrono::microseconds>(now) - std::chrono::time_point_cast<std::chrono::seconds>(now);
	const std::time_t now_c = std::chrono::system_clock::to_time_t(now);
#ifdef _MSC_VER
	std::tm tm_;
	localtime_s(&tm_, &now_c);
	const std::tm* tm = &tm_;
#else
	const std::tm* tm = std::localtime(&now_c);
#endif
	std::ostringstream ss;
	ss << std::put_time(tm, "%F %T.")
	   << std::setfill('0') << std::setw(6) << us.count()
	   << std::put_time(tm, " %z");
	return ss.str();
}

}
