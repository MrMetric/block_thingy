#include "ShaderObject.hpp"

#include <iostream>
#include <stdexcept>

#include <glad/glad.h>

#include "../../Game.hpp"
#include "../../Util.hpp"
#include "../../console/Console.hpp"

std::string get_log(const GLuint object);

ShaderObject::ShaderObject()
	:
	inited(false),
	name(0)
{
}

ShaderObject::ShaderObject(const std::string& file_path, GLenum type)
{
	(Game::instance != nullptr
		? Game::instance->console.logger
		: std::cout
	) << "compiling shader: " << file_path << "\n";

	const std::string source = Util::read_file(file_path);
	const char* source_c = source.c_str();
	const GLint source_len = static_cast<GLint>(source.length());
	name = glCreateShader(type);
	glShaderSource(name, 1, &source_c, &source_len);

	glCompileShader(name);
	GLint compile_ok;
	glGetShaderiv(name, GL_COMPILE_STATUS, &compile_ok);
	if(compile_ok == GL_FALSE)
	{
		std::string log = Util::gl_object_log(name);
		throw std::runtime_error("error compiling " + file_path + ":\n" + log);
	}

	inited = true;
}

ShaderObject::ShaderObject(ShaderObject&& that)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
		that.inited = false;
	}
}

ShaderObject::~ShaderObject()
{
	if(inited)
	{
		glDeleteShader(name);
	}
}

GLuint ShaderObject::get_name() const
{
	return name;
}
