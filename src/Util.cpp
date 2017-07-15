#include "Util.hpp"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <stdint.h>
#ifdef _WIN32
#include <shlwapi.h> // PathIsDirectory
#include <windows.h> // SetCurrentDirectoryA
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <easylogging++/easylogging++.hpp>
#include <glad/glad.h>

#include "shim/make_unique.hpp"

using std::string;
using std::to_string;
using std::unique_ptr;

bool Util::file_is_openable(const fs::path& path)
{
	return std::ifstream(path).is_open();
}

string Util::read_file(const fs::path& path)
{
	try
	{
		std::ifstream inpoot(path, std::ifstream::ate | std::ifstream::binary);
		inpoot.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		uint_fast64_t fsize = static_cast<uint_fast64_t>(inpoot.tellg());
		inpoot.seekg(0, std::ifstream::beg);
		unique_ptr<char[]> aaa = std::make_unique<char[]>(fsize);
		inpoot.read(aaa.get(), static_cast<std::streamsize>(fsize));
		string bbb(aaa.get(), fsize);
		return bbb;
	}
	// std::ios_base::failure is not always catchable: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145
	catch(const std::exception&)
	{
		LOG(ERROR) << "failed to read " << path.u8string();
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
		throw std::runtime_error("Error printing log: object is not a shader or a program");
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
		throw std::runtime_error("error changing directory to " + path.u8string() + ": " + strerror(errno));
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
	auto now = std::chrono::system_clock::now();
	auto us = std::chrono::time_point_cast<std::chrono::microseconds>(now) - std::chrono::time_point_cast<std::chrono::seconds>(now);
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm* tm = std::localtime(&now_c);
	std::stringstream ss;
	ss << std::put_time(tm, "%F %T.")
	   << std::setfill('0') << std::setw(6) << us.count()
	   << std::put_time(tm, " %z");
	return ss.str();
}
