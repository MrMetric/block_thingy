#include "Texture.hpp"

namespace Graphics::OpenGL {

Texture::Texture()
:
	type(0),
	inited(false),
	name(0)
{
}

Texture::Texture(const GLenum type)
:
	type(type)
{
	glCreateTextures(type, 1, &name);

	inited = true;
}

Texture::Texture(Texture&& that)
:
	type(that.type)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
		//that.type = 0;
		that.inited = false;
	}
}

Texture::~Texture()
{
	if(inited)
	{
		glDeleteTextures(1, &name);
	}
}

void Texture::image2D
(
	GLint level,
	GLint internal_format,
	GLsizei width,
	GLsizei height,
	GLenum format,
	GLenum data_type,
	const GLvoid* data
)
{
	glBindTexture(type, name);
	glTexImage2D(type, level, internal_format, width, height, 0, format, data_type, data);
}

void Texture::image2D_multisample
(
	GLsizei samples,
	GLenum internal_format,
	GLsizei width,
	GLsizei height,
	bool fixed_sample_locations
)
{
	glBindTexture(type, name);
	glTexImage2DMultisample(type, samples, internal_format, width, height, fixed_sample_locations);
}

void Texture::parameter(Texture::Parameter p, GLint value)
{
	if(glTextureParameteri != nullptr)
	{
		glTextureParameteri(name, static_cast<GLenum>(p), value);
	}
	else
	{
		glBindTexture(type, name);
		glTexParameteri(type, static_cast<GLenum>(p), value);
	}
}

GLuint Texture::get_name()
{
	return name;
}

} // namespace Graphics::OpenGL
