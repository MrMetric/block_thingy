#include "RenderTarget.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <string>

#include "util/logger.hpp"

namespace block_thingy::graphics {

static GLsizei get_samples(const GLsizei samples)
{
	if(samples < 0)
	{
		LOG(WARN) << "negative samples requested (" << samples << ")\n";
		return 0;
	}

	static GLint max_samples = -1;
	if(max_samples == -1)
	{
		GLint max_color_samples;
		glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &max_color_samples);
		LOG(DEBUG) << "max color texture samples: " << max_color_samples << '\n';

		GLint max_depth_samples;
		glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &max_depth_samples);
		LOG(DEBUG) << "max depth texture samples: " << max_color_samples << '\n';

		max_samples = std::min(max_color_samples, max_depth_samples);
		assert(max_samples >= 0);
	}
	if(samples > max_samples)
	{
		LOG(WARN) << samples << " samples requested, but max is " << max_samples << '\n';
		return max_samples;
	}
	return samples;
}

RenderTarget::RenderTarget(const window_size_t& window_size, GLsizei samples)
:
	samples(get_samples(samples)),
	frame_texture(this->samples > 0 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D)
{
	if(samples == 0)
	{
		frame_texture.parameter(opengl::Texture::Parameter::min_filter, GL_LINEAR);
		frame_texture.parameter(opengl::Texture::Parameter::mag_filter, GL_LINEAR);
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
	const uint32_t width = window_size.x;
	const uint32_t height = window_size.y;

	if(samples > 0)
	{
		frame_texture.image2D_multisample
		(
			samples,
			GL_RGB,				// internal format
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
