#pragma once

#include <string>
#include <unordered_map>

#include <glad/glad.h>

class Shader
{
	public:
		Shader();
		Shader(const char* path);
		Shader(const std::string& path);
		Shader(const Shader&) = delete;
		Shader(Shader&&);
		~Shader();

		Shader& operator=(const Shader&) = delete;

		GLuint get_name() const;
		GLint get_uniform_location(const std::string&) const;
		void uniform3f(const std::string&, float, float, float) const;
		void uniform4fv(const std::string&, const float*) const;
		void uniformMatrix4fv(const std::string&, const float*) const;

	private:
		bool inited;
		GLuint name;
		std::unordered_map<std::string, GLint> uniforms;
};
