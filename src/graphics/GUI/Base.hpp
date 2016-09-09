#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include "fwd/Game.hpp"
#include "fwd/event/EventManager.hpp"
#include "graphics/GUI/WidgetContainer.hpp"
#include "fwd/util/key_mods.hpp"

#include "types/window_size_t.hpp"

namespace Graphics::GUI {

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

		virtual void keypress(int key, int scancode, int action, Util::key_mods);
		virtual void charpress(char32_t, Util::key_mods);
		virtual void mousepress(int button, int action, Util::key_mods);
		virtual void mousemove(double x, double y);
		virtual void joypress(int joystick, int button, bool pressed);
		virtual void joymove(const glm::dvec2& motion);

		std::unique_ptr<Base> parent;

		Game& game;

	protected:
		virtual void draw_gui();
		virtual void update_framebuffer_size(const window_size_t&);
		WidgetContainer root;

	private:
		event_handler_id_t event_handler;
};

} // namespace Graphics::GUI
