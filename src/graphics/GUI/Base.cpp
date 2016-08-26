#include "Base.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho

#include "Game.hpp"
#include "Gfx.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"

namespace Graphics {
namespace GUI {

Base::Base(Game& game, const WidgetContainerMode root_mode)
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
	game.event_manager.unadd_handler(event_handler);
}

void Base::init()
{
}

void Base::close()
{
	game.gui->parent->init();
	game.gui = std::move(game.gui->parent);
}

void Base::draw()
{
	GLboolean depth_test;
	glGetBooleanv(GL_DEPTH_TEST, &depth_test);
	if(depth_test) glDisable(GL_DEPTH_TEST);

	GLboolean cull_face;
	glGetBooleanv(GL_CULL_FACE, &cull_face);
	if(cull_face) glDisable(GL_CULL_FACE);

	const bool wireframe = game.wireframe();
	if(wireframe) game.wireframe = false;

	root.draw();
	draw_gui();

	if(wireframe) game.wireframe = true;
	if(cull_face) glEnable(GL_CULL_FACE);
	if(depth_test) glEnable(GL_DEPTH_TEST);
}

void Base::keypress(const int key, const int scancode, const int action, const int mods)
{
	// TODO: proper input handling
	if(action == GLFW_PRESS)
	{
		if(key == GLFW_KEY_ESCAPE)
		{
			game.console.run_line("close_gui");
		}
		else if(key == GLFW_KEY_F2)
		{
			game.console.run_line("screenshot");
		}
		else if(key == GLFW_KEY_F11)
		{
			game.console.run_line("toggle_fullscreen");
		}
	}
}

void Base::mousepress(const int button, const int action, const int mods)
{
	root.mousepress(button, action, mods);
}

void Base::mousemove(const double x, const double y)
{
	root.mousemove(x, y);
}

void Base::joymove(const glm::dvec2& motion)
{
	// TODO
}

void Base::joypress(const int joystick, const int button, const bool pressed)
{
	// TODO
}

void Base::update_framebuffer_size(const window_size_t& window_size)
{
	root.update_container({0, 0}, glm::dvec2(window_size));
}

} // namespace GUI
} // namespace Graphics
