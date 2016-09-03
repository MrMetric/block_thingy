#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Graphics::OpenGL {

class ShaderProgram
{
	public:
		ShaderProgram();
		ShaderProgram(const char* path);
		ShaderProgram(const std::string& path);
		ShaderProgram(const std::vector<std::string>&, const std::string& name);
		~ShaderProgram();

		ShaderProgram(ShaderProgram&&);
		ShaderProgram(const ShaderProgram&) = delete;
		void operator=(const ShaderProgram&) = delete;

		GLuint get_name();
		GLint get_uniform_location(const std::string&) const;
		void uniform(const std::string&, float);
		void uniform(const std::string&, double);
		void uniform(const std::string&, float, float, float);
		void uniform(const std::string&, const glm::vec3&);
		void uniform(const std::string&, const glm::vec4&);
		void uniform(const std::string&, const glm::mat4&);

	private:
		bool inited;
		GLuint name;
		std::unordered_map<std::string, GLint> uniforms;
};

} // namespace Graphics::OpenGL
