#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>

#include "shader_util.hpp"
#include "Util.hpp"

void print_log(GLuint object)
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
		std::cerr << "Error printing log: object is not a shader or a program\n";
		return;
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

	std::cerr << log << "\n";
	delete[] log;
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
		std::cerr << "error compiling " << filename << "\n";
		print_log(res);
		glDeleteShader(res);
		exit(1);
	}

	//std::cout << "compiled " << filename << "\n";

	return res;
}

GLuint make_program(const std::string& path)
{
	std::stringstream ss_path;

	ss_path << path << ".vs";
	GLuint vs = compile_shader(ss_path.str(), GL_VERTEX_SHADER);

	//ss_path.clear();
	ss_path.str("");
	ss_path << path << ".fs";
	GLuint fs = compile_shader(ss_path.str(), GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);

	glLinkProgram(program);
	GLint izgud;
	glGetProgramiv(program, GL_LINK_STATUS, &izgud);
	if(!izgud)
	{
		std::cerr << "error linking program:\n";
		print_log(program);
		exit(1);
	}

	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &izgud);
	if(!izgud)
	{
		std::cerr << "program validation failed:\n";
		print_log(program);
		exit(1);
	}

	return program;
}

GLint getUniformLocation(GLuint program, const char* name)
{
	GLint location = glGetUniformLocation(program, name);
	if(location == -1)
	{
		std::cerr << "glGetUniformLocation returned -1 for " << name << "\n";
		exit(1);
	}
	return location;
}