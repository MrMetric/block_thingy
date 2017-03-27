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

#include "std_make_unique.hpp"

using std::string;
using std::to_string;
using std::unique_ptr;

// http://www.lighthouse3d.com/cg-topics/error-tracking-in-opengl/
void printOglError(const string& file, const int line, const string& func)
{
	const GLenum glErr = glGetError();
	if(glErr != GL_NO_ERROR)
	{
		LOG(ERROR) << "OpenGL error @ " << file << ":" << line << " (" << func << "): " << Util::gl_error_string(glErr);
	}
}

bool Util::file_is_openable(const string& path)
{
	return std::ifstream(path).is_open();
}

string Util::read_file(const string& path)
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
	catch(const std::ios_base::failure&)
	{
		LOG(ERROR) << "failed to read " << path;
		throw;
	}
}

string Util::gl_error_string(const GLenum code)
{
	switch(code)
	{
		case GL_NO_ERROR:
		{
			return "no error";
		}
		case GL_INVALID_ENUM:
		{
			return "invalid enum";
		}
		case GL_INVALID_VALUE:
		{
			return "invalid value";
		}
		case GL_INVALID_OPERATION:
		{
			return "invalid operation";
		}
		#ifdef GL_STACK_OVERFLOW
		case GL_STACK_OVERFLOW:
		{
			return "stack overflow";
		}
		#endif
		#ifdef GL_STACK_UNDERFLOW
		case GL_STACK_UNDERFLOW:
		{
			return "stack underflow";
		}
		#endif
		case GL_OUT_OF_MEMORY:
		{
			return "out of memory";
		}
		case GL_INVALID_FRAMEBUFFER_OPERATION:
		{
			return "invalid framebuffer operation";
		}
		#ifdef GL_CONTEXT_LOST
		case GL_CONTEXT_LOST:
		{
			return "context lost";
		}
		#endif
		#ifdef GL_TABLE_TOO_LARGE
		case GL_TABLE_TOO_LARGE:
		{
			return "table too large";
		}
		#endif
	}
	return "unknown (" + to_string(code) + ")";
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
		throw std::runtime_error("Error printing log: object is not a shader or a program\n");
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

Util::path Util::split_path(string path)
{
	string folder, file, ext;

	const std::size_t slash_pos = path.find_last_of('/');
	if(slash_pos != string::npos)
	{
		folder = path.substr(0, slash_pos);
		path = path.substr(slash_pos + 1);
	}

	const std::size_t dot_pos = path.find_last_of('.');
	if(dot_pos != string::npos)
	{
		file = path.substr(0, dot_pos);
		ext = path.substr(dot_pos + 1);
	}
	else
	{
		file = path;
	}

	return { folder, file, ext };
}

string Util::join_path(const Util::path& path_parts)
{
	string path = path_parts.folder;
	if(!path_parts.folder.empty())
	{
		path += "/";
	}
	path += path_parts.file;
	if(!path_parts.ext.empty())
	{
		path += "." + path_parts.ext;
	}
	return path;
}

void Util::change_directory(const string& path)
{
	#ifdef _WIN32
	// TODO: unicode
	if(!SetCurrentDirectoryA(path.c_str()))
	{
		throw std::runtime_error("error changing directory to " + path + ": " + to_string(GetLastError()));
	}
	#else
	if(chdir(path.c_str()) == -1)
	{
		throw std::runtime_error("error changing directory to " + path + ": " + strerror(errno) + "\n");
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
