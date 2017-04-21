#include "VertexArray.hpp"

#include "VertexBuffer.hpp"

namespace Graphics::OpenGL {

VertexArray::VertexArray(const VertexBuffer& vbo)
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
			stride += format.size;
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
		offset += format.size;
	}
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
