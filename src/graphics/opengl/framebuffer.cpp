#include "framebuffer.hpp"

#include "graphics/opengl/renderbuffer.hpp"
#include "graphics/opengl/texture.hpp"

namespace block_thingy::graphics::opengl {

framebuffer::framebuffer()
{
	glCreateFramebuffers(1, &name);

	inited = true;
}

framebuffer::~framebuffer()
{
	glDeleteFramebuffers(1, &name);
}

framebuffer::framebuffer(framebuffer&& that)
{
	name = that.name;
	inited = that.inited;
	if(that.inited)
	{
		that.name = 0;
		that.inited = false;
	}
}

void framebuffer::attach_renderbuffer(GLenum attachment_point, renderbuffer& rb)
{
	glNamedFramebufferRenderbuffer(name, attachment_point, GL_RENDERBUFFER, rb.get_name());
}

void framebuffer::attach_texture(GLenum attachment_point, texture& tex, GLint mipmap_level)
{
	glNamedFramebufferTexture(name, attachment_point, tex.get_name(), mipmap_level);
}

GLuint framebuffer::get_name()
{
	return name;
}

}
