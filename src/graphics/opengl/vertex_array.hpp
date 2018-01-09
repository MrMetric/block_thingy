#pragma once

#include <cstddef>

#include <glad/glad.h>

namespace block_thingy::graphics::opengl {

class vertex_buffer;

class vertex_array
{
public:
	// TODO: allow multiple buffers
	vertex_array(const vertex_buffer&);
	~vertex_array();

	vertex_array(vertex_array&&) noexcept;
	vertex_array(const vertex_array&) = delete;
	vertex_array& operator=(vertex_array&&) = delete;
	vertex_array& operator=(const vertex_array&) = delete;

	GLuint get_name()
	{
		return name;
	}

	void attrib(GLuint index, bool enabled);

	void draw(GLenum mode, GLint first, std::size_t count) const;

private:
	bool inited;
	GLuint name;
};

}
