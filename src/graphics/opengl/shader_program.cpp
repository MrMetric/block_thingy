#include "shader_program.hpp"

#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#ifdef BT_RELOADABLE_SHADERS
	#include <strict_variant/variant.hpp>
#endif

#include "game.hpp"
#include "resource_manager.hpp"
#include "graphics/opengl/shader_object.hpp"
#include "util/logger.hpp"
#include "util/misc.hpp"

using std::string;
using std::unique_ptr;

namespace block_thingy::graphics::opengl {

static GLuint make_program(const std::vector<GLuint>& objects, const string& debug_name);
static std::vector<string> get_uniform_names(const GLuint name);

struct shader_program::impl
{
	std::vector<resource<shader_object>> res;

#ifdef BT_RELOADABLE_SHADERS
	std::unordered_map<string, strict_variant::variant
	<
		GLint, GLuint, float, double,
		glm::ivec2, glm::ivec3, glm::ivec4, glm::uvec2, glm::uvec3, glm::uvec4,
		glm::vec2, glm::vec3, glm::vec4, glm::dvec2, glm::dvec3, glm::dvec4,
		glm::mat2, glm::mat3, glm::mat4, glm::mat2x3, glm::mat3x2, glm::mat2x4, glm::mat4x2, glm::mat3x4, glm::mat4x3,
		glm::dmat2, glm::dmat3, glm::dmat4, glm::dmat2x3, glm::dmat3x2, glm::dmat2x4, glm::dmat4x2, glm::dmat3x4, glm::dmat4x3
	>> uniform_values;
#endif

	void init(shader_program&, const string& debug_name);
};

shader_program::shader_program()
:
	inited(false),
	name(0)
{
}

shader_program::shader_program(const fs::path& path)
:
	shader_program({ { path.string() + ".vs" }, { path.string() + ".fs" } }, path.string())
{
}

shader_program::shader_program
(
	const std::vector<fs::path>& file_paths,
	const string& debug_name
)
:
	pImpl(std::make_unique<impl>())
{
#ifdef BT_RELOADABLE_SHADERS
	auto reload = [this, debug_name]()
	{
		pImpl->init(*this, debug_name);
		for(const auto& p : pImpl->uniform_values)
		{
			const string& u = p.first;
			const auto& v = p.second;
			// note: these calls will call insert_or_assign on uniform_values
			// this is unecessary, but should not cause a problem
			     if(const GLint*        x = v.get<GLint       >()) uniform(u, *x);
			else if(const GLuint*       x = v.get<GLuint      >()) uniform(u, *x);
			else if(const float*        x = v.get<float       >()) uniform(u, *x);
			else if(const double*       x = v.get<double      >()) uniform(u, *x);
			else if(const glm::ivec2*   x = v.get<glm::ivec2  >()) uniform(u, *x);
			else if(const glm::ivec3*   x = v.get<glm::ivec3  >()) uniform(u, *x);
			else if(const glm::ivec4*   x = v.get<glm::ivec4  >()) uniform(u, *x);
			else if(const glm::uvec2*   x = v.get<glm::uvec2  >()) uniform(u, *x);
			else if(const glm::uvec3*   x = v.get<glm::uvec3  >()) uniform(u, *x);
			else if(const glm::uvec4*   x = v.get<glm::uvec4  >()) uniform(u, *x);
			else if(const glm::vec2*    x = v.get<glm::vec2   >()) uniform(u, *x);
			else if(const glm::vec3*    x = v.get<glm::vec3   >()) uniform(u, *x);
			else if(const glm::vec4*    x = v.get<glm::vec4   >()) uniform(u, *x);
			else if(const glm::dvec2*   x = v.get<glm::dvec2  >()) uniform(u, *x);
			else if(const glm::dvec3*   x = v.get<glm::dvec3  >()) uniform(u, *x);
			else if(const glm::dvec4*   x = v.get<glm::dvec4  >()) uniform(u, *x);
			else if(const glm::mat2*    x = v.get<glm::mat2   >()) uniform(u, *x);
			else if(const glm::mat3*    x = v.get<glm::mat3   >()) uniform(u, *x);
			else if(const glm::mat4*    x = v.get<glm::mat4   >()) uniform(u, *x);
			else if(const glm::mat2x3*  x = v.get<glm::mat2x3 >()) uniform(u, *x);
			else if(const glm::mat3x2*  x = v.get<glm::mat3x2 >()) uniform(u, *x);
			else if(const glm::mat2x4*  x = v.get<glm::mat2x4 >()) uniform(u, *x);
			else if(const glm::mat4x2*  x = v.get<glm::mat4x2 >()) uniform(u, *x);
			else if(const glm::mat3x4*  x = v.get<glm::mat3x4 >()) uniform(u, *x);
			else if(const glm::mat4x3*  x = v.get<glm::mat4x3 >()) uniform(u, *x);
			else if(const glm::dmat2*   x = v.get<glm::dmat2  >()) uniform(u, *x);
			else if(const glm::dmat3*   x = v.get<glm::dmat3  >()) uniform(u, *x);
			else if(const glm::dmat4*   x = v.get<glm::dmat4  >()) uniform(u, *x);
			else if(const glm::dmat2x3* x = v.get<glm::dmat2x3>()) uniform(u, *x);
			else if(const glm::dmat3x2* x = v.get<glm::dmat3x2>()) uniform(u, *x);
			else if(const glm::dmat2x4* x = v.get<glm::dmat2x4>()) uniform(u, *x);
			else if(const glm::dmat4x2* x = v.get<glm::dmat4x2>()) uniform(u, *x);
			else if(const glm::dmat3x4* x = v.get<glm::dmat3x4>()) uniform(u, *x);
			else if(const glm::dmat4x3* x = v.get<glm::dmat4x3>()) uniform(u, *x);
			else assert(false);
		}
	};
#endif
	for(const auto& path : file_paths)
	{
		pImpl->res.push_back(game::instance->resource_manager.get_shader_object(path));
	#ifdef BT_RELOADABLE_SHADERS
		pImpl->res.back().on_update(reload);
	#endif
	}
	pImpl->init(*this, debug_name);

	inited = true;
}

shader_program::~shader_program()
{
	if(inited)
	{
		glDeleteProgram(name);
	}
}

shader_program::shader_program(shader_program&& that)
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

GLint shader_program::get_uniform_location(const string& name) const
{
	const auto i = uniforms.find(name);
	if(i == uniforms.cend())
	{
		return -1;
	}
	return i->second;
}

void shader_program::use() const
{
	glUseProgram(name);
}

void shader_program::uniform(const string& u, const GLint x)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, x);
#endif
	glProgramUniform1i(name, get_uniform_location(u), x);
}
void shader_program::uniform(const string& u, const GLint x, const GLint y)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::ivec2(x, y));
#endif
	glProgramUniform2i(name, get_uniform_location(u), x, y);
}
void shader_program::uniform(const string& u, const GLint x, const GLint y, const GLint z)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::ivec3(x, y, z));
#endif
	glProgramUniform3i(name, get_uniform_location(u), x, y, z);
}
void shader_program::uniform(const string& u, const GLint x, const GLint y, const GLint z, const GLint w)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::ivec4(x, y, z, w));
#endif
	glProgramUniform4i(name, get_uniform_location(u), x, y, z, w);
}

void shader_program::uniform(const string& u, const GLuint x)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, x);
#endif
	glProgramUniform1ui(name, get_uniform_location(u), x);
}
void shader_program::uniform(const string& u, const GLuint x, const GLuint y)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::uvec2(x, y));
#endif
	glProgramUniform2ui(name, get_uniform_location(u), x, y);
}
void shader_program::uniform(const string& u, const GLuint x, const GLuint y, const GLuint z)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::uvec3(x, y, z));
#endif
	glProgramUniform3ui(name, get_uniform_location(u), x, y, z);
}
void shader_program::uniform(const string& u, const GLuint x, const GLuint y, const GLuint z, const GLuint w)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::uvec4(x, y, z, w));
#endif
	glProgramUniform4ui(name, get_uniform_location(u), x, y, z, w);
}

void shader_program::uniform(const string& u, const float x)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, x);
#endif
	glProgramUniform1f(name, get_uniform_location(u), x);
}
void shader_program::uniform(const string& u, const float x, const float y)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::vec2(x, y));
#endif
	glProgramUniform2f(name, get_uniform_location(u), x, y);
}
void shader_program::uniform(const string& u, const float x, const float y, const float z)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::vec3(x, y, z));
#endif
	glProgramUniform3f(name, get_uniform_location(u), x, y, z);
}
void shader_program::uniform(const string& u, const float x, const float y, const float z, const float w)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::vec4(x, y, z, w));
#endif
	glProgramUniform4f(name, get_uniform_location(u), x, y, z, w);
}

void shader_program::uniform(const string& u, const double x)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, x);
#endif
	glProgramUniform1d(name, get_uniform_location(u), x);
}
void shader_program::uniform(const string& u, const double x, const double y)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::dvec2(x, y));
#endif
	glProgramUniform2d(name, get_uniform_location(u), x, y);
}
void shader_program::uniform(const string& u, const double x, const double y, const double z)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::dvec3(x, y, z));
#endif
	glProgramUniform3d(name, get_uniform_location(u), x, y, z);
}
void shader_program::uniform(const string& u, const double x, const double y, const double z, const double w)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, glm::dvec4(x, y, z, w));
#endif
	glProgramUniform4d(name, get_uniform_location(u), x, y, z, w);
}

void shader_program::uniform(const string& u, const glm::ivec2& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform2iv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void shader_program::uniform(const string& u, const glm::ivec3& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform3iv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void shader_program::uniform(const string& u, const glm::ivec4& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform4iv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}

void shader_program::uniform(const string& u, const glm::uvec2& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform2uiv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void shader_program::uniform(const string& u, const glm::uvec3& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform3uiv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void shader_program::uniform(const string& u, const glm::uvec4& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform4uiv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}

void shader_program::uniform(const string& u, const glm::vec2& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform2fv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void shader_program::uniform(const string& u, const glm::vec3& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform3fv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void shader_program::uniform(const string& u, const glm::vec4& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform4fv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}

void shader_program::uniform(const string& u, const glm::dvec2& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform2dv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void shader_program::uniform(const string& u, const glm::dvec3& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform3dv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}
void shader_program::uniform(const string& u, const glm::dvec4& vec)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, vec);
#endif
	glProgramUniform4dv(name, get_uniform_location(u), 1, glm::value_ptr(vec));
}

void shader_program::uniform(const string& u, const glm::mat2& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix2fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::mat3& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix3fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::mat4& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix4fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::mat2x3& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix2x3fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::mat3x2& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix3x2fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::mat2x4& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix2x4fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::mat4x2& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix4x2fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::mat3x4& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix3x4fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::mat4x3& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix4x3fv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}

void shader_program::uniform(const string& u, const glm::dmat2& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix2dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::dmat3& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix3dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::dmat4& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix4dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::dmat2x3& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix2x3dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::dmat3x2& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix3x2dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::dmat2x4& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix2x4dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::dmat4x2& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix4x2dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::dmat3x4& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix3x4dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}
void shader_program::uniform(const string& u, const glm::dmat4x3& mat)
{
#ifdef BT_RELOADABLE_SHADERS
	pImpl->uniform_values.insert_or_assign(u, mat);
#endif
	glProgramUniformMatrix4x3dv(name, get_uniform_location(u), 1, GL_FALSE, glm::value_ptr(mat));
}

void shader_program::impl::init(shader_program& p, const string& debug_name)
{
	std::vector<GLuint> objects;
	for(auto& r : res)
	{
		objects.push_back(r->get_name());
	}
	p.name = make_program(objects, debug_name);

	p.uniforms.clear();
	std::vector<string> uniform_names = get_uniform_names(p.name);
	for(const string& s : uniform_names)
	{
		GLint location = glGetUniformLocation(p.name, s.c_str());
		if(location == -1)
		{
			throw std::runtime_error("glGetUniformLocation returned -1 for " + s + " in " + debug_name);
		}
		p.uniforms.emplace(s, location);
	}
}

static GLuint make_program(const std::vector<GLuint>& objects, const string& debug_name)
{
	GLuint program = glCreateProgram();
	for(const auto object : objects)
	{
		glAttachShader(program, object);
	}

	GLint is_good;

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &is_good);
	if(!is_good)
	{
		const string log = util::gl_object_log(program);
		glDeleteProgram(program);
		throw std::runtime_error("error linking program " + debug_name + ":\n" + log);
	}

#ifdef DEBUG_BUILD
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &is_good);
	if(!is_good)
	{
		const string log = util::gl_object_log(program);
		LOG(WARN) << "program validation failed in " << debug_name << ":\n" << log << '\n';
	}
#endif

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

}
