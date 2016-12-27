#include "VertexArray.hpp"

#include "VertexBuffer.hpp"

namespace Graphics::OpenGL {

VertexArray::VertexArray(const VertexBuffer& vbo)
{
	glCreateVertexArrays(1, &name);
	inited = true;

	attrib(0, true);
	glBindVertexArray(name);
	glBindBuffer(GL_ARRAY_BUFFER, vbo.name);
	glVertexAttribPointer(
		0,
		vbo.format.size,
		vbo.format.type,
		vbo.format.normalized,
		vbo.format.stride,
		reinterpret_cast<GLvoid*>(vbo.format.offset)
	);
}

VertexArray::VertexArray(VertexArray&& that)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
		that.inited = false;
	}
}

VertexArray::~VertexArray()
{
	if(inited)
	{
		glDeleteVertexArrays(1, &name);
	}
}

GLuint VertexArray::get_name()
{
	return name;
}

void VertexArray::attrib(const GLuint index, const bool enabled)
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

void VertexArray::draw(const GLenum mode, const GLint first, const std::size_t count) const
{
	glBindVertexArray(name);
	glDrawArrays(mode, first, count);
}

} // namespace Graphics::OpenGL
