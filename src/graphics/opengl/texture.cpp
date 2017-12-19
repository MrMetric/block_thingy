#include "texture.hpp"

#include <limits>
#include <stdexcept>

namespace block_thingy::graphics::opengl {

texture::texture()
:
	type(0),
	inited(false),
	name(0)
{
}

texture::texture(const GLenum type)
:
	type(type)
{
	glCreateTextures(type, 1, &name);

	inited = true;
}

texture::texture(texture&& that)
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

texture::~texture()
{
	if(inited)
	{
		glDeleteTextures(1, &name);
	}
}

void texture::image2D
(
	const GLint level,
	const GLint internal_format,
	const uint32_t width_,
	const uint32_t height_,
	const GLenum format,
	const GLenum data_type,
	const GLvoid* data
)
{
	if(width_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("width");
	if(height_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("height");
	const GLsizei width = static_cast<GLsizei>(width_);
	const GLsizei height = static_cast<GLsizei>(height_);

	glBindTexture(type, name);
	glTexImage2D(type, level, internal_format, width, height, 0, format, data_type, data);
}

void texture::image2D_multisample
(
	const GLsizei samples,
	const GLenum internal_format,
	const uint32_t width_,
	const uint32_t height_,
	const bool fixed_sample_locations
)
{
	if(width_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("width");
	if(height_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("height");
	const GLsizei width = static_cast<GLsizei>(width_);
	const GLsizei height = static_cast<GLsizei>(height_);

	glBindTexture(type, name);
	glTexImage2DMultisample(type, samples, internal_format, width, height, fixed_sample_locations);
}

void texture::image3D
(
	const GLint level,
	const GLint internal_format,
	const uint32_t width_,
	const uint32_t height_,
	const uint32_t depth_,
	const GLenum format,
	const GLenum data_type,
	const GLvoid* data
)
{
	if(width_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("width");
	if(height_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("height");
	if(depth_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("depth");
	const GLsizei width = static_cast<GLsizei>(width_);
	const GLsizei height = static_cast<GLsizei>(height_);
	const GLsizei depth = static_cast<GLsizei>(depth_);

	glBindTexture(type, name);
	glTexImage3D(type, level, internal_format, width, height, depth, 0, format, data_type, data);
}

void texture::image3D_sub
(
	const GLint level,
	const GLint xoffset,
	const GLint yoffset,
	const GLint zoffset,
	const uint32_t width_,
	const uint32_t height_,
	const uint32_t depth_,
	const GLenum format,
	const GLenum data_type,
	const GLvoid* data
)
{
	if(width_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("width");
	if(height_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("height");
	if(depth_ > std::numeric_limits<GLsizei>::max()) throw std::invalid_argument("depth");
	const GLsizei width = static_cast<GLsizei>(width_);
	const GLsizei height = static_cast<GLsizei>(height_);
	const GLsizei depth = static_cast<GLsizei>(depth_);

	glBindTexture(type, name);
	glTexSubImage3D(type, level, xoffset, yoffset, zoffset, width, height, depth, format, data_type, data);
}

void texture::parameter(texture::Parameter p, GLint value)
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

GLuint texture::get_name()
{
	return name;
}

}
