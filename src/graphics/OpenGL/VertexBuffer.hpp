#pragma once

#include <glad/glad.h>

namespace Graphics {
namespace OpenGL {

class VertexBuffer
{
	public:
		VertexBuffer();
		~VertexBuffer();

		VertexBuffer(VertexBuffer&&);
		VertexBuffer(const VertexBuffer&) = delete;
		void operator=(const VertexBuffer&) = delete;

		enum class UsageHint : GLenum
		{
			stream_draw = GL_STREAM_DRAW,
			stream_read = GL_STREAM_READ,
			stream_copy = GL_STREAM_COPY,
			static_draw = GL_STATIC_DRAW,
			static_read = GL_STATIC_READ,
			static_copy = GL_STATIC_COPY,
			dynamic_draw = GL_DYNAMIC_DRAW,
			dynamic_read = GL_DYNAMIC_READ,
			dynamic_copy = GL_DYNAMIC_COPY,
		};

		void data(size_t size, const void* data, UsageHint usage);

		GLuint get_name();

	private:
		bool inited;
		GLuint name;
};

} // namespace OpenGL
} // namespace Graphics
