#pragma once

#include <cstddef>
#include <vector>

#include <glad/glad.h>

namespace Graphics::OpenGL {

class VertexBuffer
{
	friend class VertexArray;

	public:
		struct Format;
		VertexBuffer(Format);
		VertexBuffer(std::vector<Format>);
		~VertexBuffer();

		VertexBuffer(VertexBuffer&&);
		VertexBuffer(const VertexBuffer&) = delete;
		void operator=(const VertexBuffer&) = delete;

		GLuint get_name();

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
		void data(std::size_t size, const void* data, UsageHint usage);

		struct Format
		{
			GLint size;
			GLenum type;
			GLsizei byte_size = 0;
			bool normalized = false;
		};

	private:
		bool inited;
		GLuint name;
		std::vector<Format> formats;
};

} // namespace Graphics::OpenGL
