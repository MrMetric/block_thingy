#include "Util.hpp"

#include <GL/gl.h>

#include <fstream>
#include <iostream>

std::string Util::read_file(const std::string& path)
{
	std::ifstream inpoot(path, std::ios::ate | std::ios::binary);
	if(!inpoot.is_open())
	{
		std::cerr << "failed to read " << path << "\n";
		exit(1);
	}
	auto fsize_signed = inpoot.tellg();
	if(fsize_signed < 0)
	{
		// TODO: handle this
		std::cerr << "Failed to read " << path << "\n";
		return "";
	}
	uint_fast64_t fsize = uint_fast64_t(fsize_signed);
	inpoot.seekg(0, std::ios::beg);
	char* aaa = new char[fsize];
	inpoot.read(aaa, fsize_signed);
	std::string bbb(aaa, fsize);
	delete[] aaa;
	return bbb;
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
		case GL_STACK_OVERFLOW:
		{
			return "stack overflow";
		}
		case GL_STACK_UNDERFLOW:
		{
			return "stack underflow";
		}
		case GL_OUT_OF_MEMORY:
		{
			return "out of memory";
		}
		case GL_INVALID_FRAMEBUFFER_OPERATION:
		{
			return "invalid framebuffer operation";
		}
		case GL_TABLE_TOO_LARGE:
		{
			return "table too large";
		}
	}
	return "unknown (" + std::to_string(code) + ")";
}