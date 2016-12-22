#pragma once

#include <glad/glad.h>

namespace Graphics::OpenGL {

class Renderbuffer
{
	public:
		Renderbuffer();
		~Renderbuffer();

		Renderbuffer(Renderbuffer&&);
		Renderbuffer(const Renderbuffer&) = delete;
		void operator=(const Renderbuffer&) = delete;

		/** https://www.opengl.org/sdk/docs/man/html/glRenderbufferStorage.xhtml
		 ** https://www.opengl.org/sdk/docs/man/html/glRenderbufferStorageMultisample.xhtml
		 * Set data storage, format, dimensions, and optionally sample count
		 */
		void storage(GLenum internal_format, GLsizei width, GLsizei height, GLsizei samples = 0);

		GLuint get_name();

	private:
		GLuint name;
};

}
