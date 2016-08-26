#pragma once

#include <memory>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "fwd/Game.hpp"
#include "fwd/event/EventManager.hpp"
#include "graphics/Text.hpp"
#include "graphics/GUI/WidgetContainer.hpp"

#include "types/window_size_t.hpp"

namespace Graphics {
namespace GUI {

class Base
{
	public:
		Base(Game&, WidgetContainerMode root_mode);
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
		virtual void joypress(int joystick, int button, bool pressed);
		virtual void joymove(const glm::dvec2& motion);

		std::unique_ptr<Base> parent;

		Game& game;

	protected:
		virtual void draw_gui() = 0;
		virtual void update_framebuffer_size(const window_size_t&);
		WidgetContainer root;

	private:
		event_handler_id_t event_handler;
};

} // namespace GUI
} // namespace Graphics
