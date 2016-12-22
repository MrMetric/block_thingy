#pragma once

#include <glad/glad.h>

#include "fwd/graphics/OpenGL/Renderbuffer.hpp"
#include "fwd/graphics/OpenGL/Texture.hpp"

namespace Graphics::OpenGL {

class Framebuffer
{
	public:
		Framebuffer();
		~Framebuffer();

		Framebuffer(Framebuffer&&);
		Framebuffer(const Framebuffer&) = delete;
		void operator=(const Framebuffer&) = delete;

		/** https://www.opengl.org/sdk/docs/man/html/glFramebufferRenderbuffer.xhtml
		 * Attach a renderbuffer as a logical buffer of this framebuffer
		 */
		void attach_renderbuffer(GLenum attachment_point, Renderbuffer& renderbuffer);

		/** https://www.opengl.org/sdk/docs/man/html/glFramebufferTexture.xhtml
		 * Attach a level of a texture object as a logical buffer of this framebuffer
		 * @param level The texture's mipmap level
		 */
		void attach_texture(GLenum attachment_point, Texture& texture, GLint level);

		GLuint get_name();

	private:
		bool inited;
		GLuint name;
};

}
