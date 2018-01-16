#include "Base.hpp"

#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game.hpp"
#include "Gfx.hpp"
#include "settings.hpp"
#include "console/Console.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/opengl/push_state.hpp"
#include "input/joy_press.hpp"
#include "input/key_press.hpp"
#include "input/mouse_press.hpp"
#include "util/gui_parser.hpp"
#include "util/logger.hpp"
#include "util/misc.hpp"

using std::string;

namespace block_thingy::graphics::gui {

Base::Base
(
	game& g,
	const fs::path& layout_path
)
:
	g(g),
	root(nullptr)
{
	if(!layout_path.empty())
	{
		root.read_layout(util::parse_gui(layout_path));
	}
	event_handler = g.event_manager.add_handler(EventType::window_size_change, [this](const Event& event)
	{
		const auto& e = static_cast<const Event_window_size_change&>(event);
		update_framebuffer_size(e.window_size);
	});
	update_framebuffer_size(g.gfx.window_size);
}

Base::~Base()
{
	try
	{
		g.event_manager.unadd_handler(event_handler);
	}
	catch(const std::runtime_error& e)
	{
		// should never happen, but it is best to be safe (destructors are noexcept)
		LOG(BUG) << "error unadding event handler in graphics::gui::Base destructor: " << e.what() << '\n';
	}
}

void Base::switch_to()
{
	double x, y;
	glfwGetCursorPos(g.gfx.window, &x, &y);
	mousemove({x, y});
}

void Base::switch_from()
{
}

void Base::close()
{
	g.close_gui();
}

void Base::draw()
{
	opengl::push_state<GLboolean, GL_DEPTH_TEST> _depth_test(false);

	root.draw();
	draw_gui();
}

void Base::keypress(const input::key_press& press)
{
	// TODO: proper input handling
	if(press.action == GLFW_PRESS)
	{
		if(press.key == GLFW_KEY_ESCAPE)
		{
			Console::instance->run_line("close_gui");
			return;
		}
		if(press.key == GLFW_KEY_F2)
		{
			Console::instance->run_line("screenshot");
			return;
		}
		if(press.key == GLFW_KEY_F11)
		{
			Console::instance->run_line("toggle_bool fullscreen");
			return;
		}
	}
	if(press.key == 1000) // TODO
	{
		double x, y;
		glfwGetCursorPos(g.gfx.window, &x, &y);
		mousepress({{x, y}, GLFW_MOUSE_BUTTON_LEFT, press.action, 0});
		return;
	}
	root.keypress(press);
}

void Base::charpress(const input::char_press& press)
{
	root.charpress(press);
}

void Base::mousepress(const input::mouse_press& press)
{
	root.mousepress(press);
}

void Base::mousemove(const glm::dvec2& position)
{
	root.mousemove(position);
}

void Base::joymove(const glm::dvec2& offset)
{
	double x, y;
	glfwGetCursorPos(g.gfx.window, &x, &y);
	const double mouse_speed = settings::get<double>("joystick_mouse_speed");
	x += offset.x * mouse_speed;
	y += offset.y * mouse_speed;
	glfwSetCursorPos(g.gfx.window, x, y);
	mousemove(glm::dvec2(x, y));
}

void Base::draw_gui()
{
}

void Base::update_framebuffer_size(const window_size_t& window_size)
{
	rhea::simplex_solver solver;

	widget::Base::style_vars_t window_vars;
	solver.add_constraints
	({
		window_vars["pos.x"   ] == 0                         | rhea::strength::required(),
		window_vars["pos.y"   ] == 0                         | rhea::strength::required(),
		window_vars["size.x"  ] == window_size.x             | rhea::strength::required(),
		window_vars["size.y"  ] == window_size.y             | rhea::strength::required(),
		window_vars["end.x"   ] == window_vars["size.x"]     | rhea::strength::required(),
		window_vars["end.y"   ] == window_vars["size.y"]     | rhea::strength::required(),
		window_vars["center.x"] == window_vars["size.x"] / 2 | rhea::strength::required(),
		window_vars["center.y"] == window_vars["size.y"] / 2 | rhea::strength::required(),
	});

	root.apply_layout(solver, root, window_vars);
	root.use_layout();
}

}
