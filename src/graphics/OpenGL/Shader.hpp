#pragma once

#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

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
		void uniform1f(const std::string&, float) const;
		void uniform3f(const std::string&, float, float, float) const;
		void uniform4fv(const std::string&, const glm::vec4&) const;
		void uniformMatrix4fv(const std::string&, const glm::mat4&) const;

	private:
		bool inited;
		GLuint name;
		std::unordered_map<std::string, GLint> uniforms;
};
