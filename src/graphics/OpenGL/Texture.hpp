#pragma once

#include <stdint.h>

#include <glad/glad.h>

namespace Graphics::OpenGL {

class Texture
{
	public:
		Texture();
		Texture(GLenum type, const GLvoid*, uint_fast32_t width, uint_fast32_t height, GLenum data_type);
		~Texture();

		Texture(Texture&&);
		Texture(const Texture&) = delete;
		void operator=(const Texture&) = delete;

		GLuint get_name();

	private:
		bool inited;
		GLuint name;
};

} // namespace Graphics::OpenGL
