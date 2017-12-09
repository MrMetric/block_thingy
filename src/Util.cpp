#include "Util.hpp"

#include <algorithm>
#include <cstddef>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h> // SetCurrentDirectoryW
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <glad/glad.h>

#include "util/logger.hpp"

using std::string;
using std::to_string;
using std::unique_ptr;

bool Util::file_is_openable(const fs::path& path)
{
	return std::ifstream(path).is_open();
}

string Util::read_text(const fs::path& path)
{
	return read_file(path, true);
}

string Util::read_file(const fs::path& path)
{
	return read_file(path, false);
}

string Util::read_file(const fs::path& path, const bool is_text)
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
		inpoot.seekg(0, std::ifstream::beg);
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

string Util::gl_object_log(const GLuint object)
{
	GLint log_length;
	if(glIsShader(object))
	{
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	}
	else if(glIsProgram(object))
	{
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	}
	else
	{
		throw std::runtime_error("can not get log for an OpenGL object that is not a shader or a program");
	}

	unique_ptr<char[]> log = std::make_unique<char[]>(static_cast<std::size_t>(log_length));

	if(glIsShader(object))
	{
		glGetShaderInfoLog(object, log_length, nullptr, log.get());
	}
	else if(glIsProgram(object))
	{
		glGetProgramInfoLog(object, log_length, nullptr, log.get());
	}

	return string(log.get());
}

void Util::change_directory(const fs::path& path)
{
	#ifdef _WIN32
	if(!SetCurrentDirectoryW(path.wstring().c_str()))
	{
		throw std::runtime_error("error changing directory to " + path.u8string() + ": " + to_string(GetLastError()));
	}
	#else
	if(chdir(path.c_str()) == -1)
	{
		throw std::runtime_error("error changing directory to " + path.u8string() + ": " + std::strerror(errno));
	}
	#endif
}

int Util::stoi(const string& s)
{
	if(s.find_first_not_of("0123456789") != string::npos)
	{
		throw std::invalid_argument("stoi");
	}
	return std::stoi(s);
}

string Util::datetime()
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
