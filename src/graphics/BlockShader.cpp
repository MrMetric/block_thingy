#include "BlockShader.hpp"

#include "../shader_util.hpp"

BlockShader::BlockShader() : inited(false) {}

BlockShader::BlockShader(const char* path) : BlockShader(std::string(path)) {}

BlockShader::BlockShader(const std::string& path)
{
	program = make_program(path);
	inited = true;
	uniforms.emplace("matriks", getUniformLocation(program, "matriks"));
	uniforms.emplace("pos_mod", getUniformLocation(program, "pos_mod"));
}

BlockShader::BlockShader(BlockShader&& that)
{
	inited = that.inited;
	if(inited)
	{
		program = that.program;
		uniforms = std::move(that.uniforms);
		that.inited = false;
	}
}

BlockShader::~BlockShader()
{
	if(inited)
	{
		glDeleteProgram(program);
	}
}

GLint BlockShader::get_uniform_location(const std::string& name) const
{
	auto i = uniforms.find(name);
	if(i == uniforms.end())
	{
		return -1;
	}
	return i->second;
}

void BlockShader::uniform3f(const std::string& name, const float x, const float y, const float z) const
{
	const GLint u = get_uniform_location(name);
	glUniform3f(u, x, y, z);
}

void BlockShader::uniformMatrix4fv(const std::string& name, const float* ptr) const
{
	const GLint u = get_uniform_location(name);
	glUniformMatrix4fv(u, 1, GL_FALSE, ptr);
}
