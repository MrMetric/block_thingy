#include "Util.hpp"

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <memory>

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

std::string Util::gl_error_string(const uint_fast32_t code)
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
		/*
		case GL_STACK_OVERFLOW:
		{
			return "stack overflow";
		}
		case GL_STACK_UNDERFLOW:
		{
			return "stack underflow";
		}
		*/
		case GL_OUT_OF_MEMORY:
		{
			return "out of memory";
		}
		case GL_INVALID_FRAMEBUFFER_OPERATION:
		{
			return "invalid framebuffer operation";
		}
		/*
		case GL_TABLE_TOO_LARGE:
		{
			return "table too large";
		}
		*/
	}
	return "unknown (" + std::to_string(code) + ")";
}
