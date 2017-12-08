#pragma once

#include "graphics/OpenGL/Framebuffer.hpp"
#include "graphics/OpenGL/Renderbuffer.hpp"
#include "graphics/OpenGL/Texture.hpp"
#include "types/window_size_t.hpp"

namespace Graphics {

class RenderTarget
{
	GLsizei samples; // needs to be initialized before frame_texture

public:
	RenderTarget(const window_size_t&, GLsizei samples = 0);

	OpenGL::Framebuffer frame_buffer;
	OpenGL::Renderbuffer depth_buffer;
	OpenGL::Texture frame_texture;

	void resize(const window_size_t&);
};

}
