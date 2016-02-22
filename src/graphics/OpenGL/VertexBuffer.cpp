#include "VertexBuffer.hpp"

VertexBuffer::VertexBuffer()
{
	glCreateBuffers(1, &vbo);
	inited = true;
}

VertexBuffer::VertexBuffer(VertexBuffer&& that)
{
	inited = that.inited;
	if(inited)
	{
		vbo = that.vbo;
		that.inited = false;
	}
}

VertexBuffer::~VertexBuffer()
{
	if(inited)
	{
		glDeleteBuffers(1, &vbo);
	}
}

void VertexBuffer::data(GLsizei size, const void* data, GLenum usage)
{
	glNamedBufferData(vbo, size, data, usage);
}

GLuint VertexBuffer::get_name() const
{
	return vbo;
}
