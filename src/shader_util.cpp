#include "shader_util.hpp"

#include <stdexcept>

#include <GL/glew.h>

#include "Util.hpp"

std::string get_log(GLuint object)
{
	GLuint log_length = 0;
	if(glIsShader(object))
	{
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, reinterpret_cast<GLint*>(&log_length));
	}
	else if(glIsProgram(object))
	{
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, reinterpret_cast<GLint*>(&log_length));
	}
	else
	{
		throw std::runtime_error("Error printing log: object is not a shader or a program\n");
	}

	char* log = new char[log_length];

	if(glIsShader(object))
	{
		glGetShaderInfoLog(object, log_length, nullptr, log);
	}
	else if(glIsProgram(object))
	{
		glGetProgramInfoLog(object, log_length, nullptr, log);
	}

	std::string log_string(log);
	delete[] log;
	return log_string;
}

GLuint compile_shader(const std::string& filename, GLenum type)
{
	const std::string source = Util::read_file(filename);
	const char* source_c = source.c_str();
	const GLint source_len = GLint(source.length()); // I don't think the length will ever be that large (2 GiB) anyway
	GLuint res = glCreateShader(type);
	glShaderSource(res, 1, &source_c, &source_len);

	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if(compile_ok == GL_FALSE)
	{
		std::string log = get_log(res);
		glDeleteShader(res);
		throw std::runtime_error("error compiling " + filename + ":\n" + log);
	}

	return res;
}

GLuint make_program(const std::string& path)
{
	GLuint vs = compile_shader(path + ".vs", GL_VERTEX_SHADER);
	GLuint fs = compile_shader(path + ".fs", GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);

	GLint izgud;

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &izgud);
	if(!izgud)
	{
		std::string log = get_log(program);
		glDeleteShader(vs);
		glDeleteShader(fs);
		glDeleteProgram(program);
		throw std::runtime_error("error linking program:\n" + log);
	}

	// shader objects are not needed after linking, so memory can be freed by deleting them
	glDetachShader(program, vs);
	glDeleteShader(vs);
	glDetachShader(program, fs);
	glDeleteShader(fs);

	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &izgud);
	if(!izgud)
	{
		std::string log = get_log(program);
		glDeleteProgram(program);
		throw std::runtime_error("program validation failed:\n" + log);
	}

	return program;
}

GLint getUniformLocation(GLuint program, const char* name)
{
	GLint location = glGetUniformLocation(program, name);
	if(location == -1)
	{
		throw std::runtime_error(std::string("glGetUniformLocation returned -1 for ") + name);
	}
	return location;
}