#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>
#include <glm/mat2x2.hpp>
#include <glm/mat2x3.hpp>
#include <glm/mat2x4.hpp>
#include <glm/mat3x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x4.hpp>
#include <glm/mat4x2.hpp>
#include <glm/mat4x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "resource_manager.hpp"
#include "shim/propagate_const.hpp"
#include "util/filesystem.hpp"

namespace block_thingy::graphics::opengl {

class shader_program
{
public:
	shader_program();
	shader_program(const fs::path&);
	shader_program(const std::vector<fs::path>&, const std::string& debug_name);
	~shader_program();

	shader_program(shader_program&&);
	shader_program(const shader_program&) = delete;
	shader_program& operator=(shader_program&&) = delete;
	shader_program& operator=(const shader_program&) = delete;

	GLuint get_name()
	{
		return name;
	}
	GLint get_uniform_location(const std::string&) const;
	void use() const;

	// https://www.opengl.org/sdk/docs/man/html/glUniform.xhtml
	// https://www.opengl.org/registry/specs/ARB/gpu_shader_fp64.txt
	void uniform(const std::string&, GLint);
	void uniform(const std::string&, GLint, GLint);
	void uniform(const std::string&, GLint, GLint, GLint);
	void uniform(const std::string&, GLint, GLint, GLint, GLint);

	void uniform(const std::string&, GLuint);
	void uniform(const std::string&, GLuint, GLuint);
	void uniform(const std::string&, GLuint, GLuint, GLuint);
	void uniform(const std::string&, GLuint, GLuint, GLuint, GLuint);

	void uniform(const std::string&, float);
	void uniform(const std::string&, float, float);
	void uniform(const std::string&, float, float, float);
	void uniform(const std::string&, float, float, float, float);

	void uniform(const std::string&, double);
	void uniform(const std::string&, double, double);
	void uniform(const std::string&, double, double, double);
	void uniform(const std::string&, double, double, double, double);

	void uniform(const std::string&, const glm::ivec2&);
	void uniform(const std::string&, const glm::ivec3&);
	void uniform(const std::string&, const glm::ivec4&);

	void uniform(const std::string&, const glm::uvec2&);
	void uniform(const std::string&, const glm::uvec3&);
	void uniform(const std::string&, const glm::uvec4&);

	void uniform(const std::string&, const glm::vec2&);
	void uniform(const std::string&, const glm::vec3&);
	void uniform(const std::string&, const glm::vec4&);

	void uniform(const std::string&, const glm::dvec2&);
	void uniform(const std::string&, const glm::dvec3&);
	void uniform(const std::string&, const glm::dvec4&);

	void uniform(const std::string&, const glm::mat2&);
	void uniform(const std::string&, const glm::mat3&);
	void uniform(const std::string&, const glm::mat4&);
	void uniform(const std::string&, const glm::mat2x3&);
	void uniform(const std::string&, const glm::mat3x2&);
	void uniform(const std::string&, const glm::mat2x4&);
	void uniform(const std::string&, const glm::mat4x2&);
	void uniform(const std::string&, const glm::mat3x4&);
	void uniform(const std::string&, const glm::mat4x3&);

	void uniform(const std::string&, const glm::dmat2&);
	void uniform(const std::string&, const glm::dmat3&);
	void uniform(const std::string&, const glm::dmat4&);
	void uniform(const std::string&, const glm::dmat2x3&);
	void uniform(const std::string&, const glm::dmat3x2&);
	void uniform(const std::string&, const glm::dmat2x4&);
	void uniform(const std::string&, const glm::dmat4x2&);
	void uniform(const std::string&, const glm::dmat3x4&);
	void uniform(const std::string&, const glm::dmat4x3&);

private:
	bool inited;
	GLuint name;
	std::unordered_map<std::string, GLint> uniforms;

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
