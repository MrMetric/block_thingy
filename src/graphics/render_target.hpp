#pragma once

#include "graphics/opengl/framebuffer.hpp"
#include "graphics/opengl/renderbuffer.hpp"
#include "graphics/opengl/texture.hpp"
#include "types/window_size_t.hpp"

namespace block_thingy::graphics {

class render_target
{
	GLsizei samples; // needs to be initialized before frame_texture

public:
	render_target(const window_size_t&, GLsizei samples = 0);

	opengl::framebuffer frame_buffer;
	opengl::renderbuffer depth_buffer;
	opengl::texture frame_texture;

	void resize(const window_size_t&);
};

}
