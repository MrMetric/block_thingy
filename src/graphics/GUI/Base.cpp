#include "Base.hpp"

#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <json.hpp>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"
#include "console/Console.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/OpenGL/PushState.hpp"
#include "util/key_press.hpp"
#include "util/logger.hpp"

using std::string;

namespace block_thingy::graphics::gui {

Base::Base
(
	game& g,
	const fs::path& layout_path
)
:
	g(g)
{
	if(!layout_path.empty())
	{
		root.read_layout(json::parse(util::read_file(layout_path)));
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

void Base::init()
{
}

void Base::close()
{
	g.close_gui();
}

void Base::draw()
{
	opengl::push_state<GLboolean> depth_test(GL_DEPTH_TEST, false);
	opengl::push_state<GLboolean> cull_face(GL_CULL_FACE, false);

	root.draw();
	draw_gui();
}

void Base::keypress(const util::key_press& press)
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
	root.keypress(press);
}

void Base::charpress(const util::char_press& press)
{
	root.charpress(press);
}

void Base::mousepress(const util::mouse_press& press)
{
	root.mousepress(press);
}

void Base::mousemove(const double x, const double y)
{
	root.mousemove(x, y);
}

void Base::joymove(const glm::dvec2& motion)
{
	double x, y;
	glfwGetCursorPos(g.gfx.window, &x, &y);
	x += motion.x * 12.0;
	y += motion.y * 12.0;
	glfwSetCursorPos(g.gfx.window, x, y);
	mousemove(x, y);
}

void Base::joypress(const int joystick, const int button, const bool pressed)
{
	// TODO
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
		window_vars["pos.x"] == 0,
		window_vars["pos.y"] == 0,
		window_vars["size.x"] == window_size.x,
		window_vars["size.y"] == window_size.y,
		window_vars["end.x"] == window_vars["size.x"],
		window_vars["end.y"] == window_vars["size.y"],
		window_vars["center.x"] == window_vars["size.x"] / 2,
		window_vars["center.y"] == window_vars["size.y"] / 2,
	});

	root.apply_layout(solver, window_vars);
	root.use_layout();
}

}
