#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "fwd/Game.hpp"
#include "graphics/Text.hpp"

#include "types/window_size_t.hpp"

namespace Graphics {
namespace GUI {

class Base
{
	public:
		Base(Game&);
		virtual ~Base();

		Base(Base&&) = delete;
		Base(const Base&) = delete;
		void operator=(const Base&) = delete;

		virtual void init();
		virtual void close();
		virtual void draw();

		virtual void keypress(int key, int scancode, int action, int mods);
		virtual void mousepress(int button, int action, int mods);
		virtual void mousemove(double x, double y);

		std::unique_ptr<Base> parent;

	protected:
		Game& game;

		virtual void draw_gui() = 0;
		virtual void update_framebuffer_size(const window_size_t&) = 0;
};

} // namespace GUI
} // namespace Graphics
