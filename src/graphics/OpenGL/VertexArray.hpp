#pragma once

#include <cstddef>

#include <glad/glad.h>

namespace block_thingy::graphics::opengl {

class VertexBuffer;

class VertexArray
{
public:
	// TODO: allow multiple buffers
	VertexArray(const VertexBuffer&);
	~VertexArray();

	VertexArray(VertexArray&&);
	VertexArray(const VertexArray&) = delete;
	VertexArray& operator=(VertexArray&&) = delete;
	VertexArray& operator=(const VertexArray&) = delete;

	GLuint get_name();

	void attrib(GLuint index, bool enabled);

	void draw(GLenum mode, GLint first, std::size_t count) const;

private:
	bool inited;
	GLuint name;
};

}
