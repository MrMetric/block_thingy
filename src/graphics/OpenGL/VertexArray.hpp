#pragma once

#include <glad/glad.h>

namespace Graphics {
namespace OpenGL {

class VertexBuffer;

class VertexArray
{
	public:
		// TODO: allow multiple buffers
		VertexArray(const VertexBuffer&);
		~VertexArray();

		VertexArray(VertexArray&&);
		VertexArray(const VertexArray&) = delete;
		void operator=(const VertexArray&) = delete;

		GLuint get_name();

		void attrib(GLuint index, bool enabled);

		void draw(GLenum mode, GLint first, size_t count) const;

	private:
		bool inited;
		GLuint name;
};

} // namespace OpenGL
} // namespace Graphics
