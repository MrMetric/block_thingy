#pragma once

#include "graphics/OpenGL/Framebuffer.hpp"
#include "graphics/OpenGL/Renderbuffer.hpp"
#include "graphics/OpenGL/Texture.hpp"
#include "types/window_size_t.hpp"

namespace block_thingy::graphics {

class RenderTarget
{
	GLsizei samples; // needs to be initialized before frame_texture

public:
	RenderTarget(const window_size_t&, GLsizei samples = 0);

	opengl::Framebuffer frame_buffer;
	opengl::Renderbuffer depth_buffer;
	opengl::Texture frame_texture;

	void resize(const window_size_t&);
};

}
