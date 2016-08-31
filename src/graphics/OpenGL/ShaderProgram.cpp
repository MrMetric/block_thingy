#include "ShaderProgram.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderObject.hpp"

#include "Util.hpp"

#include "std_make_unique.hpp"

using std::string;
using std::unique_ptr;

namespace Graphics {
namespace OpenGL {

static GLuint make_program(const std::vector<string>& files, const string& debug_name);
static std::vector<string> get_uniform_names(const GLuint name);

ShaderProgram::ShaderProgram()
	:
	inited(false),
	name(0)
{
}

ShaderProgram::ShaderProgram(const char* path)
	:
	ShaderProgram(string(path))
{
}

ShaderProgram::ShaderProgram(const string& path)
	:
	ShaderProgram({ path + ".vs", path + ".fs" }, path)
{
}

ShaderProgram::ShaderProgram(const std::vector<string>& files, const string& debug_name)
{
	name = make_program(files, debug_name);

	std::vector<string> uniform_names = get_uniform_names(name);
	for(const string& s : uniform_names)
	{
		GLint location = glGetUniformLocation(name, s.c_str());
		if(location == -1)
		{
			throw std::runtime_error("glGetUniformLocation returned -1 for " + s + " in " + debug_name);
		}
		uniforms.emplace(s, location);
	}

	inited = true;
}

ShaderProgram::ShaderProgram(ShaderProgram&& that)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
		uniforms = std::move(that.uniforms);
		that.inited = false;
	}
}

ShaderProgram::~ShaderProgram()
{
	if(inited)
	{
		glDeleteProgram(name);
	}
}

GLuint ShaderProgram::get_name()
{
	return name;
}

GLint ShaderProgram::get_uniform_location(const string& name) const
{
	const auto i = uniforms.find(name);
	if(i == uniforms.cend())
	{
		return -1;
	}
	return i->second;
}

void ShaderProgram::uniform(const string& uniform_name, const float x)
{
	const GLint u = get_uniform_location(uniform_name);
	glProgramUniform1f(name, u, x);
}

void ShaderProgram::uniform(const string& uniform_name, const double x)
{
	const GLint u = get_uniform_location(uniform_name);
	glProgramUniform1d(name, u, x);
}

void ShaderProgram::uniform(const string& uniform_name, const float x, const float y, const float z)
{
	const GLint u = get_uniform_location(uniform_name);
	glProgramUniform3f(name, u, x, y, z);
}

void ShaderProgram::uniform(const string& uniform_name, const glm::vec3& vec)
{
	const GLint u = get_uniform_location(uniform_name);
	const float* ptr = glm::value_ptr(vec);
	glProgramUniform3fv(name, u, 1, ptr);
}

void ShaderProgram::uniform(const string& uniform_name, const glm::vec4& vec)
{
	const GLint u = get_uniform_location(uniform_name);
	const float* ptr = glm::value_ptr(vec);
	glProgramUniform4fv(name, u, 1, ptr);
}

void ShaderProgram::uniform(const string& uniform_name, const glm::mat4& matrix)
{
	const GLint u = get_uniform_location(uniform_name);
	const float* ptr = glm::value_ptr(matrix);
	glProgramUniformMatrix4fv(name, u, 1, GL_FALSE, ptr);
}

static GLuint make_program(const std::vector<string>& files, const string& debug_name)
{
	std::vector<ShaderObject> objects;
	for(string path : files)
	{
		GLenum type;
		Util::path path_parts = Util::split_path(path);
		if(path_parts.ext == "vs")
		{
			type = GL_VERTEX_SHADER;
		}
		else if(path_parts.ext ==  "fs")
		{
			type = GL_FRAGMENT_SHADER;
		}
		else
		{
			throw std::invalid_argument("bad shader path: " + path);
		}
		if(!Util::file_is_openable(path))
		{
			path_parts.file = "default";
			const string path2 = Util::join_path(path_parts);
			if(!Util::file_is_openable(path2))
			{
				throw std::runtime_error("shader not found and no default exists: " + path);
			}
			path = path2;
		}
		objects.emplace_back(path, type);
	}

	// TODO: attach in previous loop?
	GLuint program = glCreateProgram();
	for(ShaderObject& object : objects)
	{
		glAttachShader(program, object.get_name());
	}

	GLint izgud;

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &izgud);
	if(izgud == GL_FALSE)
	{
		string log = Util::gl_object_log(program);
		glDeleteProgram(program);
		throw std::runtime_error("error linking program " + debug_name + ":\n" + log);
	}

	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &izgud);
	if(izgud == GL_FALSE)
	{
		string log = Util::gl_object_log(program);
		glDeleteProgram(program);
		throw std::runtime_error("program validation failed in " + debug_name + ":\n" + log);
	}

	// shader objects are not needed after linking, so memory can be freed by deleting them
	// but a shader object will not be deleted until it is detached
	for(ShaderObject& object : objects)
	{
		glDetachShader(program, object.get_name());
	}

	return program;
}

static std::vector<string> get_uniform_names(const GLuint program)
{
	GLint uniform_count;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);

	GLint max_name_length;
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_length);

	std::vector<string> uniform_names;
	for(GLint i = 0; i < uniform_count; ++i)
	{
		unique_ptr<char[]> uniform_name = std::make_unique<char[]>(static_cast<std::size_t>(max_name_length));
		GLsizei name_length;
		glGetActiveUniformName(program, i, static_cast<GLsizei>(max_name_length), &name_length, uniform_name.get());
		uniform_names.emplace_back(uniform_name.get(), name_length);
	}

	return uniform_names;
}

} // namespace OpenGL
} // namespace Graphics
