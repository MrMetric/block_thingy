#pragma once

#include <unordered_map>
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

		GLint get_uniform_location(const std::string&) const;
		void uniform3f(const std::string&, float, float, float) const;
		void uniformMatrix4fv(const std::string&, const float*) const;

		GLuint program;

	private:
		bool inited;
		std::unordered_map<std::string, GLint> uniforms;
};
