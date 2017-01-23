#include "ShaderProgram.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "ShaderObject.hpp"

#include "ResourceManager.hpp"
#include "Util.hpp"

#include "std_make_unique.hpp"

using std::string;
using std::unique_ptr;

namespace Graphics::OpenGL {

static GLuint make_program(const std::vector<GLuint>& objects, const string& debug_name);
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

ShaderProgram::ShaderProgram
(
	const std::vector<string>& file_paths,
	const string& debug_name
)
:
	debug_name(debug_name)
{
	for(const string& path : file_paths)
	{
		res.push_back(ResourceManager::get_ShaderObject(path));
		res.back().on_update([this]()
		{
			init();
		});
	}
	init();

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

void ShaderProgram::uniform(const string& u, const GLint x)
{
	glProgramUniform1i(name, get_uniform_location(u), x);
}
void ShaderProgram::uniform(const string& u, const GLint x, const GLint y)
{
	glProgramUniform2i(name, get_uniform_location(u), x, y);
}
void ShaderProgram::uniform(const string& u, const GLint x, const GLint y, const GLint z)
{
	glProgramUniform3i(name, get_uniform_location(u), x, y, z);
}
void ShaderProgram::uniform(const string& u, const GLint x, const GLint y, const GLint z, const GLint w)
{
	glProgramUniform4i(name, get_uniform_location(u), x, y, z, w);
}

void ShaderProgram::uniform(const string& u, const GLuint x)
{
	glProgramUniform1ui(name, get_uniform_location(u), x);
}
void ShaderProgram::uniform(const string& u, const GLuint x, const GLuint y)
{
	glProgramUniform2ui(name, get_uniform_location(u), x, y);
}
void ShaderProgram::uniform(const string& u, const GLuint x, const GLuint y, const GLuint z)
{
	glProgramUniform3ui(name, get_uniform_location(u), x, y, z);
}
void ShaderProgram::uniform(const string& u, const GLuint x, const GLuint y, const GLuint z, const GLuint w)
{
	glProgramUniform4ui(name, get_uniform_location(u), x, y, z, w);
}

void ShaderProgram::uniform(const string& u, const float x)
{
	glProgramUniform1f(name, get_uniform_location(u), x);
}
void ShaderProgram::uniform(const string& u, const float x, const float y)
{
	glProgramUniform2f(name, get_uniform_location(u), x, y);
}
void ShaderProgram::uniform(const string& u, const float x, const float y, const float z)
{
	glProgramUniform3f(name, get_uniform_location(u), x, y, z);
}
void ShaderProgram::uniform(const string& u, const float x, const float y, const float z, const float w)
{
	glProgramUniform4f(name, get_uniform_location(u), x, y, z, w);
}

void ShaderProgram::uniform(const string& u, const double x)
{
	glProgramUniform1d(name, get_uniform_location(u), x);
}
void ShaderProgram::uniform(const string& u, const double x, const double y)
{
	glProgramUniform2d(name, get_uniform_location(u), x, y);
}
void ShaderProgram::uniform(const string& u, const double x, const double y, const double z)
{
	glProgramUniform3d(name, get_uniform_location(u), x, y, z);
}
void ShaderProgram::uniform(const string& u, const double x, const double y, const double z, const double w)
{
	glProgramUniform4d(name, get_uniform_location(u), x, y, z, w);
}

void ShaderProgram::uniform(const string& u, const glm::ivec2& vec)
{
	glProgramUniform2iv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void ShaderProgram::uniform(const string& u, const glm::ivec3& vec)
{
	glProgramUniform3iv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void ShaderProgram::uniform(const string& u, const glm::ivec4& vec)
{
	glProgramUniform4iv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}

void ShaderProgram::uniform(const string& u, const glm::uvec2& vec)
{
	glProgramUniform2uiv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void ShaderProgram::uniform(const string& u, const glm::uvec3& vec)
{
	glProgramUniform3uiv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void ShaderProgram::uniform(const string& u, const glm::uvec4& vec)
{
	glProgramUniform4uiv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}

void ShaderProgram::uniform(const string& u, const glm::vec2& vec)
{
	glProgramUniform2fv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void ShaderProgram::uniform(const string& u, const glm::vec3& vec)
{
	glProgramUniform3fv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void ShaderProgram::uniform(const string& u, const glm::vec4& vec)
{
	glProgramUniform4fv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}

void ShaderProgram::uniform(const string& u, const glm::dvec2& vec)
{
	glProgramUniform2dv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void ShaderProgram::uniform(const string& u, const glm::dvec3& vec)
{
	glProgramUniform3dv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void ShaderProgram::uniform(const string& u, const glm::dvec4& vec)
{
	glProgramUniform4dv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}

void ShaderProgram::uniform(const string& u, const glm::mat2& mat)
{
	glProgramUniformMatrix2fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::mat3& mat)
{
	glProgramUniformMatrix3fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::mat4& mat)
{
	glProgramUniformMatrix4fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::mat2x3& mat)
{
	glProgramUniformMatrix2x3fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::mat3x2& mat)
{
	glProgramUniformMatrix3x2fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::mat2x4& mat)
{
	glProgramUniformMatrix2x4fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::mat4x2& mat)
{
	glProgramUniformMatrix4x2fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::mat3x4& mat)
{
	glProgramUniformMatrix3x4fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::mat4x3& mat)
{
	glProgramUniformMatrix4x3fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}

void ShaderProgram::uniform(const string& u, const glm::dmat2& mat)
{
	glProgramUniformMatrix2dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::dmat3& mat)
{
	glProgramUniformMatrix3dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::dmat4& mat)
{
	glProgramUniformMatrix4dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::dmat2x3& mat)
{
	glProgramUniformMatrix2x3dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::dmat3x2& mat)
{
	glProgramUniformMatrix3x2dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::dmat2x4& mat)
{
	glProgramUniformMatrix2x4dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::dmat4x2& mat)
{
	glProgramUniformMatrix4x2dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::dmat3x4& mat)
{
	glProgramUniformMatrix3x4dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::uniform(const string& u, const glm::dmat4x3& mat)
{
	glProgramUniformMatrix4x3dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}

void ShaderProgram::init()
{
	std::vector<GLuint> objects;
	for(auto& r : res)
	{
		objects.push_back(r->get_name());
	}
	name = make_program(objects, debug_name);

	uniforms.clear();
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
}

static GLuint make_program(const std::vector<GLuint>& objects, const string& debug_name)
{
	GLuint program = glCreateProgram();
	for(const auto object : objects)
	{
		glAttachShader(program, object);
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
	for(GLuint object : objects)
	{
		glDetachShader(program, object);
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

} // namespace Graphics::OpenGL
