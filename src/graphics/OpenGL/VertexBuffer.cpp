#include "VertexBuffer.hpp"

#include <glad/glad.h>

namespace Graphics {
namespace OpenGL {

VertexBuffer::VertexBuffer(const Format& format)
	:
	format(format)
{
	glCreateBuffers(1, &name);
	inited = true;
}

VertexBuffer::VertexBuffer(VertexBuffer&& that)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
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

GLuint VertexBuffer::get_name()
{
	return name;
}

void VertexBuffer::data(const std::size_t size, const void* data, const UsageHint usage)
{
	glNamedBufferData(name, size, data, static_cast<GLenum>(usage));
}

} // namespace OpenGL
} // namespace Graphics
