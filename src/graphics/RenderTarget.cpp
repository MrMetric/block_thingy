#include "RenderTarget.hpp"

#include <stdexcept>
#include <string>

namespace Graphics {

RenderTarget::RenderTarget(const window_size_t& window_size, GLsizei samples)
:
	frame_texture(samples > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D),
	samples(samples)
{
	if(samples <= 0)
	{
		frame_texture.parameter(OpenGL::Texture::Parameter::min_filter, GL_LINEAR);
		frame_texture.parameter(OpenGL::Texture::Parameter::mag_filter, GL_LINEAR);
	}

	resize(window_size);

	frame_buffer.attach_renderbuffer(GL_DEPTH_ATTACHMENT, depth_buffer);
	frame_buffer.attach_texture(GL_COLOR_ATTACHMENT0, frame_texture, 0);

	GLenum status = glCheckNamedFramebufferStatus(frame_buffer.get_name(), GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("RenderTarget's framebuffer status is " + std::to_string(status));
	}
}

void RenderTarget::resize(const window_size_t& window_size)
{
	GLsizei width = static_cast<GLsizei>(window_size.x);
	GLsizei height = static_cast<GLsizei>(window_size.y);

	if(samples > 0)
	{
		frame_texture.image2D_multisample
		(
			samples,
			GL_RGB,
			width,
			height,
			true
		);
	}
	else
	{
		frame_texture.image2D
		(
			0,					// level; 0 = not a mipmap
			GL_RGB,				// internal format
			width,
			height,
			GL_RGB,				// format
			GL_UNSIGNED_BYTE,	// data type
			nullptr				// data
		);
	}

	depth_buffer.storage(GL_DEPTH_COMPONENT, width, height, samples);
}

}
