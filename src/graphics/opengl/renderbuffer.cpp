#include "renderbuffer.hpp"

#include <limits>
#include <stdexcept>

namespace block_thingy::graphics::opengl {

renderbuffer::renderbuffer()
{
	glCreateRenderbuffers(1, &name);
}

renderbuffer::~renderbuffer()
{
	glDeleteRenderbuffers(1, &name);
}

void renderbuffer::storage
(
	const GLenum internal_format,
	const uint32_t width_,
	const uint32_t height_,
	const GLsizei samples
)
{
	if(width_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("width");
	if(height_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("height");
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

GLuint renderbuffer::get_name()
{
	return name;
}

}
