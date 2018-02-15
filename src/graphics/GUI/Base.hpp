#pragma once

#include <memory>
#include <string>

#include <glm/vec2.hpp>

#include "fwd/game.hpp"
#include "fwd/event/EventManager.hpp"
#include "graphics/GUI/Widget/Container.hpp"
#include "fwd/input/char_press.hpp"
#include "fwd/input/joy_press.hpp"
#include "fwd/input/key_press.hpp"
#include "fwd/input/mouse_press.hpp"
#include "types/window_size_t.hpp"
#include "util/filesystem.hpp"

namespace block_thingy::graphics::gui {

class Base
{
public:
	Base
	(
		game&,
		const fs::path& layout_path
	);
	virtual ~Base();

	Base(Base&&) = delete;
	Base(const Base&) = delete;
	Base& operator=(Base&&) = delete;
	Base& operator=(const Base&) = delete;

	virtual std::string type() const = 0;

	virtual void switch_to();
	virtual void switch_from();
	virtual void close();
	virtual void draw();
	void refresh_layout();

	virtual void keypress(const input::key_press&);
	virtual void charpress(const input::char_press&);
	virtual void mousepress(const input::mouse_press&);
	virtual void mousemove(const glm::dvec2& position);
	virtual void joymove(const glm::dvec2& offset);

	std::unique_ptr<Base> parent;

	game& g;

protected:
	virtual void draw_gui();
	virtual void update_framebuffer_size(const window_size_t&);
	widget::Container root;

private:
	event_handler_id_t event_handler;
};

}
