#pragma once

#include <string>

#include <glad/glad.h>

#include "util/filesystem.hpp"

namespace block_thingy::graphics::opengl {

class shader_object
{
public:
	shader_object();
	shader_object(const fs::path&, GLenum type);
	~shader_object();

	shader_object(shader_object&&) noexcept;
	shader_object(const shader_object&) = delete;
	shader_object& operator=(shader_object&&) = delete;
	shader_object& operator=(const shader_object&) = delete;

	GLuint get_name()
	{
		return name;
	}

private:
	bool inited;
	GLuint name;
};

}
