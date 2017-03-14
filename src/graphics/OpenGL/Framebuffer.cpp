#include "Framebuffer.hpp"

#include "graphics/OpenGL/Renderbuffer.hpp"
#include "graphics/OpenGL/Texture.hpp"

namespace Graphics::OpenGL {

Framebuffer::Framebuffer()
{
	glCreateFramebuffers(1, &name);

	inited = true;
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &name);
}

Framebuffer::Framebuffer(Framebuffer&& that)
{
	name = that.name;
	inited = that.inited;
	if(that.inited)
	{
		that.name = 0;
		that.inited = false;
	}
}

void Framebuffer::attach_renderbuffer(GLenum attachment_point, Renderbuffer& renderbuffer)
{
	glNamedFramebufferRenderbuffer(name, attachment_point, GL_RENDERBUFFER, renderbuffer.get_name());
}

void Framebuffer::attach_texture(GLenum attachment_point, Texture& texture, GLint mipmap_level)
{
	glNamedFramebufferTexture(name, attachment_point, texture.get_name(), mipmap_level);
}

GLuint Framebuffer::get_name()
{
	return name;
}

};
