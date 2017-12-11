#pragma once

#include <memory>
#include <string>

#include <glm/vec2.hpp>

#include "fwd/Game.hpp"
#include "fwd/event/EventManager.hpp"
#include "graphics/GUI/Widget/Container.hpp"
#include "types/window_size_t.hpp"
#include "fwd/util/char_press.hpp"
#include "fwd/util/key_press.hpp"
#include "fwd/util/mouse_press.hpp"
#include "util/filesystem.hpp"

namespace Graphics::GUI {

class Base
{
public:
	Base
	(
		Game&,
		const fs::path& layout_path
	);
	virtual ~Base();

	Base(Base&&) = delete;
	Base(const Base&) = delete;
	Base& operator=(Base&&) = delete;
	Base& operator=(const Base&) = delete;

	virtual std::string type() const = 0;

	virtual void init();
	virtual void close();
	virtual void draw();

	virtual void keypress(const Util::key_press&);
	virtual void charpress(const Util::char_press&);
	virtual void mousepress(const Util::mouse_press&);
	virtual void mousemove(double x, double y);
	virtual void joypress(int joystick, int button, bool pressed);
	virtual void joymove(const glm::dvec2& motion);

	std::unique_ptr<Base> parent;

	Game& game;

protected:
	virtual void draw_gui();
	virtual void update_framebuffer_size(const window_size_t&);
	Widget::Container root;

private:
	event_handler_id_t event_handler;
};

}
