#pragma once

#include <cstddef>
#include <vector>

#include <glad/glad.h>

namespace block_thingy::graphics::opengl {

class vertex_buffer
{
public:
	struct Format;
	vertex_buffer(Format);
	vertex_buffer(std::vector<Format>);
	~vertex_buffer();

	vertex_buffer(vertex_buffer&&);
	vertex_buffer(const vertex_buffer&) = delete;
	vertex_buffer& operator=(vertex_buffer&&) = delete;
	vertex_buffer& operator=(const vertex_buffer&) = delete;

	GLuint get_name();

	enum class usage_hint : GLenum
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
	void data(std::size_t size, const void* data, usage_hint);

	struct Format
	{
		GLint size;
		GLenum type;
		bool normalized = false;
		GLsizei byte_size = 0;
	};

private:
	friend class vertex_array;

	bool inited;
	GLuint name;
	std::vector<Format> formats;
};

}
