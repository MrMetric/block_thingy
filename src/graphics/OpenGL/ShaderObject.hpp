#pragma once

#include <string>

#include <glad/glad.h>

#include "util/filesystem.hpp"

namespace Graphics::OpenGL {

class ShaderObject
{
public:
	ShaderObject();
	ShaderObject(const fs::path&, GLenum type);
	~ShaderObject();

	ShaderObject(ShaderObject&&);
	ShaderObject(const ShaderObject&) = delete;
	void operator=(const ShaderObject&) = delete;

	GLuint get_name();

private:
	bool inited;
	GLuint name;
};

} // namespace Graphics::OpenGL
