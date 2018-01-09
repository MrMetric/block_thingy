#include "vertex_array.hpp"

#include "graphics/opengl/vertex_buffer.hpp"

namespace block_thingy::graphics::opengl {

vertex_array::vertex_array(const vertex_buffer& vbo)
{
	glCreateVertexArrays(1, &name);
	inited = true;

	glBindVertexArray(name);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.name);

	GLsizei stride = 0;
	if(vbo.formats.size() > 1)
	{
		for(const auto& format : vbo.formats)
		{
			stride += format.byte_size;
		}
	}

	GLuint i = 0;
	GLsizeiptr offset = 0;
	for(const auto& format : vbo.formats)
	{
		glVertexAttribPointer
		(
			i,
			format.size,
			format.type,
			format.normalized,
			stride,
			reinterpret_cast<GLvoid*>(offset)
		);
		attrib(i, true);
		++i;
		offset += format.byte_size;
	}
}

vertex_array::vertex_array(vertex_array&& that) noexcept
:
	inited(that.inited),
	name(that.name)
{
	if(that.inited)
	{
		that.inited = false;
		that.name = 0;
	}
}

vertex_array::~vertex_array()
{
	if(inited)
	{
		glDeleteVertexArrays(1, &name);
	}
}

void vertex_array::attrib(const GLuint index, const bool enabled)
{
	if(enabled)
	{
		glEnableVertexArrayAttrib(name, index);
	}
	else
	{
		glDisableVertexArrayAttrib(name, index);
	}
}

void vertex_array::draw(const GLenum mode, const GLint first, const std::size_t count) const
{
	glBindVertexArray(name);
	glDrawArrays(mode, first, count);
}

}
