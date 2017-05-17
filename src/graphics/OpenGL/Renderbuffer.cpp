#include "Renderbuffer.hpp"

#include <limits>
#include <stdexcept>

namespace Graphics::OpenGL {

Renderbuffer::Renderbuffer()
{
	glCreateRenderbuffers(1, &name);
}

Renderbuffer::~Renderbuffer()
{
	glDeleteRenderbuffers(1, &name);
}

void Renderbuffer::storage
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

GLuint Renderbuffer::get_name()
{
	return name;
}

}
