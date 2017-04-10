#include "Base.hpp"

#include <utility>

#include <easylogging++/easylogging++.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "console/Console.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "util/key_mods.hpp"

namespace Graphics::GUI {

Base::Base
(
	Game& game,
	const WidgetContainerMode root_mode
)
:
	game(game),
	root(game, root_mode)
{
	event_handler = game.event_manager.add_handler(EventType::window_size_change, [this](const Event& event)
	{
		auto e = static_cast<const Event_window_size_change&>(event);
		update_framebuffer_size(e.window_size);
	});
	update_framebuffer_size(game.gfx.window_size);
}

Base::~Base()
{
	try
	{
		game.event_manager.unadd_handler(event_handler);
	}
	catch(const std::runtime_error& e)
	{
		// should never happen, but best to be safe
		LOG(ERROR) << "error unadding GUI event handler: " << e.what();
	}
}

void Base::init()
{
}

void Base::close()
{
	game.close_gui();
}

void Base::draw()
{
	GLboolean depth_test;
	glGetBooleanv(GL_DEPTH_TEST, &depth_test);
	if(depth_test) glDisable(GL_DEPTH_TEST);

	GLboolean cull_face;
	glGetBooleanv(GL_CULL_FACE, &cull_face);
	if(cull_face) glDisable(GL_CULL_FACE);

	root.draw();
	draw_gui();

	if(cull_face) glEnable(GL_CULL_FACE);
	if(depth_test) glEnable(GL_DEPTH_TEST);
}

void Base::keypress(const int key, const int scancode, const int action, const Util::key_mods mods)
{
	// TODO: proper input handling
	if(action == GLFW_PRESS)
	{
		if(key == GLFW_KEY_ESCAPE)
		{
			Console::instance->run_line("close_gui");
			return;
		}
		if(key == GLFW_KEY_F2)
		{
			Console::instance->run_line("screenshot");
			return;
		}
		if(key == GLFW_KEY_F11)
		{
			Console::instance->run_line("toggle_fullscreen");
			return;
		}
	}
	root.keypress(key, scancode, action, mods);
}

void Base::charpress(const char32_t codepoint, const Util::key_mods mods)
{
	root.charpress(codepoint, mods);
}

void Base::mousepress(const int button, const int action, const Util::key_mods mods)
{
	root.mousepress(button, action, mods);
}

void Base::mousemove(const double x, const double y)
{
	root.mousemove(x, y);
}

void Base::joymove(const glm::dvec2& motion)
{
	double x, y;
	glfwGetCursorPos(game.gfx.window, &x, &y);
	x += motion.x * 12.0;
	y += motion.y * 12.0;
	glfwSetCursorPos(game.gfx.window, x, y);
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
	root.update_container({0, 0}, glm::dvec2(window_size));
}

} // namespace Graphics::GUI
