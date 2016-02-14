#include "BlockShader.hpp"

#include "../shader_util.hpp"

BlockShader::BlockShader() : inited(false) {}

BlockShader::BlockShader(const char* path) : BlockShader(std::string(path)) {}

BlockShader::BlockShader(const std::string& path)
{
	program = make_program(path);
	inited = true;
	u_matriks = getUniformLocation(program, "matriks");
	u_pos_mod = getUniformLocation(program, "pos_mod");
}

BlockShader::BlockShader(BlockShader&& that)
{
	inited = that.inited;
	if(inited)
	{
		program = that.program;
		u_matriks = that.u_matriks;
		u_pos_mod = that.u_pos_mod;
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
