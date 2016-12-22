#include "Renderbuffer.hpp"

namespace Graphics::OpenGL {

Renderbuffer::Renderbuffer()
{
	glCreateRenderbuffers(1, &name);
}

Renderbuffer::~Renderbuffer()
{
	glDeleteRenderbuffers(1, &name);
}

void Renderbuffer::storage(GLenum internal_format, GLsizei width, GLsizei height, GLsizei samples)
{
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
