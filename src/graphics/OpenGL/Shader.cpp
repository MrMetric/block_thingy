#include "Shader.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "../../Util.hpp"

#include "std_make_unique.hpp"

static GLuint make_program(const std::string& path);
static GLuint compile_shader(const std::string& filename, GLenum type);
static std::string get_log(const GLuint object);
static std::vector<std::string> get_uniform_names(GLuint name);

Shader::Shader()
	:
	inited(false)
{
}

Shader::Shader(const char* path)
	:
	Shader(std::string(path))
{
}

Shader::Shader(const std::string& path)
{
	name = make_program(path);

	std::vector<std::string> uniform_names = get_uniform_names(name);
	for(const std::string& s : uniform_names)
	{
		GLint location = glGetUniformLocation(name, s.c_str());
		if(location == -1)
		{
			throw std::runtime_error("glGetUniformLocation returned -1 for " + s + " in " + path);
		}
		uniforms.emplace(s, location);
	}

	inited = true;
}

Shader::Shader(Shader&& that)
{
	inited = that.inited;
	if(inited)
	{
		name = that.name;
		uniforms = std::move(that.uniforms);
		that.inited = false;
	}
}

Shader::~Shader()
{
	if(inited)
	{
		glDeleteProgram(name);
	}
}

GLuint Shader::get_name() const
{
	return name;
}

GLint Shader::get_uniform_location(const std::string& name) const
{
	auto i = uniforms.find(name);
	if(i == uniforms.end())
	{
		return -1;
	}
	return i->second;
}

void Shader::uniform3f(const std::string& uniform_name, const float x, const float y, const float z) const
{
	const GLint u = get_uniform_location(uniform_name);
	glProgramUniform3f(name, u, x, y, z);
}

void Shader::uniform4fv(const std::string& uniform_name, const glm::vec4& vec) const
{
	const GLint u = get_uniform_location(uniform_name);
	const float* ptr = glm::value_ptr(vec);
	glProgramUniform4fv(name, u, 1, ptr);
}

void Shader::uniformMatrix4fv(const std::string& uniform_name, const glm::mat4& matrix) const
{
	const GLint u = get_uniform_location(uniform_name);
	const float* ptr = glm::value_ptr(matrix);
	glProgramUniformMatrix4fv(name, u, 1, GL_FALSE, ptr);
}

static GLuint make_program(const std::string& path)
{
	std::cout << "compiling shader: " << path << "\n";

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

static GLuint compile_shader(const std::string& filename, GLenum type)
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

static std::string get_log(const GLuint object)
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

static std::vector<std::string> get_uniform_names(GLuint program)
{
	GLint uniform_count;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);

	GLint max_name_length;
	glGetProgramiv(program,  GL_ACTIVE_UNIFORM_MAX_LENGTH , &max_name_length);

	std::vector<std::string> uniform_names;
	for(GLint i = 0; i < uniform_count; ++i)
	{
		std::unique_ptr<char[]> uniform_name = std::make_unique<char[]>(max_name_length);
		GLsizei name_length;
		glGetActiveUniformName(program, i, max_name_length, &name_length, uniform_name.get());
		uniform_names.emplace_back(uniform_name.get(), name_length);
	}

	return uniform_names;
}
