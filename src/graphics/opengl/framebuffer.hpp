#pragma once

#include <glad/glad.h>

#include "fwd/graphics/opengl/renderbuffer.hpp"
#include "fwd/graphics/opengl/texture.hpp"

namespace block_thingy::graphics::opengl {

class framebuffer
{
public:
	framebuffer();
	~framebuffer();

	framebuffer(framebuffer&&);
	framebuffer(const framebuffer&) = delete;
	framebuffer& operator=(framebuffer&&) = delete;
	framebuffer& operator=(const framebuffer&) = delete;

	/** https://www.opengl.org/sdk/docs/man/html/glFramebufferRenderbuffer.xhtml\n
	 * Attach a renderbuffer as a logical buffer of this framebuffer
	 */
	void attach_renderbuffer(GLenum attachment_point, renderbuffer&);

	/** https://www.opengl.org/sdk/docs/man/html/glFramebufferTexture.xhtml\n
	 * Attach a level of a texture object as a logical buffer of this framebuffer
	 */
	void attach_texture(GLenum attachment_point, texture&, GLint mipmap_level);

	GLuint get_name();

private:
	bool inited;
	GLuint name;
};

}
