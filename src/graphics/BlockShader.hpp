#pragma once

#include <string>

#include <glad/glad.h>

class BlockShader
{
	public:
		BlockShader();
		BlockShader(const char* path);
		BlockShader(const std::string& path);
		BlockShader(const BlockShader&) = delete;
		BlockShader(BlockShader&& that);
		~BlockShader();

		BlockShader& operator=(const BlockShader&) = delete;

		GLuint program;
		GLint u_matriks;
		GLint u_pos_mod;

	private:
		bool inited;
};
