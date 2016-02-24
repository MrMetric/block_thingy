#include "VertexBuffer.hpp"

#include <glad/glad.h>

VertexBuffer::VertexBuffer()
{
	glCreateBuffers(1, &name);
	inited = true;
}

VertexBuffer::VertexBuffer(VertexBuffer&& that)
{
	inited = that.inited;
	if(inited)
	{
		name = that.name;
		that.inited = false;
	}
}

VertexBuffer::~VertexBuffer()
{
	if(inited)
	{
		glDeleteBuffers(1, &name);
		inited = false;
	}
}

void VertexBuffer::data(GLsizei size, const void* data, GLenum usage)
{
	glNamedBufferData(name, size, data, usage);
}

GLuint VertexBuffer::get_name() const
{
	return name;
}
