#pragma once

#include <glad/glad.h>

class VertexBuffer
{
	public:
		VertexBuffer();
		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer(VertexBuffer&& that);
		~VertexBuffer();

		VertexBuffer& operator=(const VertexBuffer&) = delete;

		void data(GLsizei size, const void* data, GLenum usage);

		GLuint get_name() const;

	private:
		bool inited;
		GLuint vbo;
};
