#include "framebuffer.hpp"

#include "graphics/opengl/renderbuffer.hpp"
#include "graphics/opengl/texture.hpp"

namespace block_thingy::graphics::opengl {

framebuffer::framebuffer()
:
	inited(true)
{
	glCreateFramebuffers(1, &name);
}

framebuffer::~framebuffer()
{
	glDeleteFramebuffers(1, &name);
}

framebuffer::framebuffer(framebuffer&& that) noexcept
:
	inited(that.inited),
	name(that.name)
{
	if(that.inited)
	{
		that.name = 0;
		that.inited = false;
	}
}

void framebuffer::attach_renderbuffer
(
	const GLenum attachment_point,
	renderbuffer& rb
)
{
	glNamedFramebufferRenderbuffer(name, attachment_point, GL_RENDERBUFFER, rb.get_name());
}

void framebuffer::attach_texture
(
	const GLenum attachment_point,
	texture& tex,
	const GLint mipmap_level
)
{
	glNamedFramebufferTexture(name, attachment_point, tex.get_name(), mipmap_level);
}

}
