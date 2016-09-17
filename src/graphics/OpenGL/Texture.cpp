#include "Texture.hpp"

namespace Graphics::OpenGL {

Texture::Texture()
:
	inited(false),
	name(0)
{
}

Texture::Texture(const GLenum type, const GLvoid* data, const uint_fast32_t width, const uint_fast32_t height, const GLenum data_type)
{
	glCreateTextures(type, 1, &name);

	glBindTexture(type, name);
	glTexImage2D(
		type,
		0,			// level
		GL_RED,		// internal format
		width,
		height,
		0,
		GL_RED,		// format
		data_type,
		data
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	inited = true;
}

Texture::Texture(Texture&& that)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
		that.inited = false;
	}
}

Texture::~Texture()
{
	glDeleteTextures(1, &name);
}

GLuint Texture::get_name()
{
	return name;
}

} // namespace Graphics::OpenGL
