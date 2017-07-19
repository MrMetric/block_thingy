#pragma once

#include <glad/glad.h>

#include "fwd/graphics/OpenGL/Renderbuffer.hpp"
#include "fwd/graphics/OpenGL/Texture.hpp"

namespace Graphics::OpenGL {

class Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();

	Framebuffer(Framebuffer&&);
	Framebuffer(const Framebuffer&) = delete;
	void operator=(const Framebuffer&) = delete;

	/** https://www.opengl.org/sdk/docs/man/html/glFramebufferRenderbuffer.xhtml\n
	 * Attach a renderbuffer as a logical buffer of this framebuffer
	 */
	void attach_renderbuffer(GLenum attachment_point, Renderbuffer& renderbuffer);

	/** https://www.opengl.org/sdk/docs/man/html/glFramebufferTexture.xhtml\n
	 * Attach a level of a texture object as a logical buffer of this framebuffer
	 */
	void attach_texture(GLenum attachment_point, Texture& texture, GLint mipmap_level);

	GLuint get_name();

private:
	bool inited;
	GLuint name;
};

}
