#pragma once

#include <stdint.h>

#include <glad/glad.h>

namespace Graphics::OpenGL {

class Texture
{
public:
	Texture();
	Texture(GLenum type);
	~Texture();

	Texture(Texture&&);
	Texture(const Texture&) = delete;
	void operator=(const Texture&) = delete;

	// https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
	void image2D
	(
		GLint level,
		GLint internal_format,
		uint32_t width,
		uint32_t height,
		GLenum format,
		GLenum data_type,
		const GLvoid* data
	);

	// https://www.opengl.org/sdk/docs/man/html/glTexImage2DMultisample.xhtml
	void image2D_multisample
	(
		GLsizei samples,
		GLenum internal_format,
		uint32_t width,
		uint32_t height,
		bool fixed_sample_locations
	);

	void image3D
	(
		GLint level,
		GLint internal_format,
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		GLenum format,
		GLenum data_type,
		const GLvoid* data
	);

	void image3D_sub
	(
		GLint level,
		GLint xoffset,
		GLint yoffset,
		GLint zoffset,
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		GLenum format,
		GLenum data_type,
		const GLvoid* data
	);

	// https://www.opengl.org/sdk/docs/man4/html/glTexParameter.xhtml
	enum class Parameter : GLenum
	{
		base_level = GL_TEXTURE_BASE_LEVEL,
		border_color = GL_TEXTURE_BORDER_COLOR,
		compare_func = GL_TEXTURE_COMPARE_FUNC,
		compare_mode = GL_TEXTURE_COMPARE_MODE,
		lod_bias = GL_TEXTURE_LOD_BIAS,
		min_filter = GL_TEXTURE_MIN_FILTER,
		mag_filter = GL_TEXTURE_MAG_FILTER,
		min_lod = GL_TEXTURE_MIN_LOD,
		max_lod = GL_TEXTURE_MAX_LOD,
		max_level = GL_TEXTURE_MAX_LEVEL,
		swizzle_r = GL_TEXTURE_SWIZZLE_R,
		swizzle_g = GL_TEXTURE_SWIZZLE_G,
		swizzle_b = GL_TEXTURE_SWIZZLE_B,
		swizzle_a = GL_TEXTURE_SWIZZLE_A,
		wrap_s = GL_TEXTURE_WRAP_S,
		wrap_t = GL_TEXTURE_WRAP_T,
		wrap_r = GL_TEXTURE_WRAP_R,
	};
	void parameter(Parameter, GLint value);

	GLuint get_name();

	const GLenum type;

private:
	bool inited;
	GLuint name;
};

}
