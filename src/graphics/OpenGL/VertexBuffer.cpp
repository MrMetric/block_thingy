#include "VertexBuffer.hpp"

#include <stdexcept>
#include <string>
#include <utility>

#include <glad/glad.h>

namespace Graphics::OpenGL {

VertexBuffer::VertexBuffer(Format format)
:
	VertexBuffer(std::vector<Format>{std::move(format)})
{
}

VertexBuffer::VertexBuffer(std::vector<Format> formats)
:
	formats(std::move(formats))
{
	glCreateBuffers(1, &name);

	auto get_size = [](const GLenum type) -> GLsizei
	{
		switch(type)
		{
			case GL_BYTE:
			case GL_UNSIGNED_BYTE:
				return 1;
			case GL_SHORT:
			case GL_UNSIGNED_SHORT:
			case GL_HALF_FLOAT:
				return 2;
			case GL_INT:
			case GL_UNSIGNED_INT:
			//case GL_FIXED:
			case GL_FLOAT:
				return 4;
			case GL_DOUBLE:
				return 8;
		}
		throw std::runtime_error("unknown OpenGL type: " + std::to_string(type));
	};
	for(Format& f : this->formats)
	{
		f.byte_size = f.size * get_size(f.type);
	}

	inited = true;
}

VertexBuffer::~VertexBuffer()
{
	if(inited)
	{
		glDeleteBuffers(1, &name);
	}
}

VertexBuffer::VertexBuffer(VertexBuffer&& that)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
		that.inited = false;
		formats = std::move(that.formats);
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

}
