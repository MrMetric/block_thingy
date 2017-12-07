#pragma once

#include <cstddef>

#include <glad/glad.h>

namespace Graphics::OpenGL {

class VertexBuffer;

class VertexArray
{
public:
	// TODO: allow multiple buffers
	VertexArray(const VertexBuffer&);
	~VertexArray();

	VertexArray(VertexArray&&);
	VertexArray(const VertexArray&) = delete;
	void operator=(const VertexArray&) = delete;

	GLuint get_name();

	void attrib(GLuint index, bool enabled);

	void draw(GLenum mode, GLint first, std::size_t count) const;

private:
	bool inited;
	GLuint name;
};

}
