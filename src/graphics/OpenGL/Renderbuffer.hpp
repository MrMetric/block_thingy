#pragma once

#include <stdint.h>

#include <glad/glad.h>

namespace block_thingy::graphics::opengl {

class renderbuffer
{
public:
	renderbuffer();
	~renderbuffer();

	renderbuffer(renderbuffer&&);
	renderbuffer(const renderbuffer&) = delete;
	renderbuffer& operator=(renderbuffer&&) = delete;
	renderbuffer& operator=(const renderbuffer&) = delete;

	/** https://www.opengl.org/sdk/docs/man/html/glRenderbufferStorage.xhtml
	 ** https://www.opengl.org/sdk/docs/man/html/glRenderbufferStorageMultisample.xhtml
	 * Set data storage, format, dimensions, and optionally sample count
	 */
	void storage
	(
		GLenum internal_format,
		uint32_t width,
		uint32_t height,
		GLsizei samples = 0
	);

	GLuint get_name();

private:
	GLuint name;
};

}
