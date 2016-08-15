#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Graphics {
namespace OpenGL {

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

		GLuint get_name() const;
		GLint get_uniform_location(const std::string&) const;
		void uniform(const std::string&, float) const;
		void uniform(const std::string&, double) const;
		void uniform(const std::string&, float, float, float) const;
		void uniform(const std::string&, const glm::vec3&) const;
		void uniform(const std::string&, const glm::vec4&) const;
		void uniform(const std::string&, const glm::mat4&) const;

	private:
		bool inited;
		GLuint name;
		std::unordered_map<std::string, GLint> uniforms;
};

} // namespace OpenGL
} // namespace Graphics
