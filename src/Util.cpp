#include "Util.hpp"

#include <cerrno>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <unistd.h>

#include <glad/glad.h>

#include "std_make_unique.hpp"

bool Util::file_is_openable(const std::string& path)
{
	return std::ifstream(path).is_open();
}

std::string Util::read_file(const std::string& path)
{
	try
	{
		std::ifstream inpoot(path, std::ios::ate | std::ios::binary);
		inpoot.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		uint_fast64_t fsize = static_cast<uint_fast64_t>(inpoot.tellg());
		inpoot.seekg(0, std::ios::beg);
		std::unique_ptr<char[]> aaa = std::make_unique<char[]>(fsize);
		inpoot.read(aaa.get(), static_cast<std::streamsize>(fsize));
		std::string bbb(aaa.get(), fsize);
		return bbb;
	}
	catch(const std::ios_base::failure&)
	{
		std::cerr << "failed to read " << path << "\n";
		throw;
	}
}

std::string Util::gl_error_string(const GLenum code)
{
	switch(code)
	{
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
	return "unknown (" + std::to_string(code) + ")";
}

std::string Util::gl_object_log(const GLuint object)
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

	std::unique_ptr<char[]> log = std::make_unique<char[]>(static_cast<size_t>(log_length));

	if(glIsShader(object))
	{
		glGetShaderInfoLog(object, log_length, nullptr, log.get());
	}
	else if(glIsProgram(object))
	{
		glGetProgramInfoLog(object, log_length, nullptr, log.get());
	}

	std::string log_string(log.get());
	return log_string;
}

Util::path Util::split_path(const std::string& path)
{
	auto slash_pos = path.find_last_of('/');
	auto dot_pos = path.find_last_of('.');
	std::string folder = path.substr(0, slash_pos);
	std::string file = path.substr(slash_pos + 1, dot_pos - slash_pos - 1);
	std::string ext = path.substr(dot_pos + 1);
	return { folder, file, ext };
}

std::string Util::join_path(const Util::path& path_parts)
{
	std::string path = path_parts.folder + "/" + path_parts.file;
	if(path_parts.ext != "")
	{
		path += "." + path_parts.ext;
	}
	return path;
}

void Util::change_directory(const std::string& path)
{
	// TODO: find out what Wandows uses
	if(chdir(path.c_str()) == -1)
	{
		throw std::runtime_error("error changing directory to " + path + ": " + strerror(errno) + "\n");
	}
}

double Util::mod(double x, double y)
{
	return x - (std::floor(x / y) * y);
}
