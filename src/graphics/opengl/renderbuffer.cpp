#include "renderbuffer.hpp"

#include <limits>
#include <stdexcept>

namespace block_thingy::graphics::opengl {

renderbuffer::renderbuffer()
:
	inited(true)
{
	glCreateRenderbuffers(1, &name);
}

renderbuffer::~renderbuffer()
{
	if(inited)
	{
		glDeleteRenderbuffers(1, &name);
	}
}

renderbuffer::renderbuffer(renderbuffer&& that) noexcept
:
	inited(that.inited),
	name(that.name)
{
	if(that.inited)
	{
		that.inited = false;
		that.name = 0;
	}
}

void renderbuffer::storage
(
	const GLenum internal_format,
	const uint32_t width_,
	const uint32_t height_,
	const GLsizei samples
)
{
	static_assert(sizeof(GLsizei) <= sizeof(uint32_t));
	if(width_ > static_cast<uint32_t>(std::numeric_limits<GLsizei>::max()))
	{
		throw std::invalid_argument("width out of range");
	}
	if(height_ > static_cast<uint32_t>(std::numeric_limits<GLsizei>::max()))
	{
		throw std::invalid_argument("height out of range");
	}
	const GLsizei width = static_cast<GLsizei>(width_);
	const GLsizei height = static_cast<GLsizei>(height_);

	if(samples > 0)
	{
		glNamedRenderbufferStorageMultisample(name, samples, internal_format, width, height);
	}
	else
	{
		glNamedRenderbufferStorage(name, internal_format, width, height);
	}
}

}
