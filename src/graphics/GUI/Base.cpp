#include "Base.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::ortho

#include "Game.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"

namespace Graphics {
namespace GUI {

Base::Base(Game& game)
	:
	game(game)
{
	game.event_manager.add_handler(EventType::window_size_change, [this](const Event& event)
	{
		auto e = static_cast<const Event_window_size_change&>(event);
		update_framebuffer_size(e.window_size);
	});
}

Base::~Base()
{
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

	draw_gui();

	if(wireframe) game.wireframe = true;
	if(cull_face) glEnable(GL_CULL_FACE);
	if(depth_test) glEnable(GL_DEPTH_TEST);
}

void Base::keypress(const int key, const int scancode, const int action, const int mods)
{
	if(action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
	{
		game.console.run_line("close_gui");
	}
}

void Base::mousepress(const int button, const int action, const int mods)
{
}

void Base::mousemove(const double x, const double y)
{
}

} // namespace GUI
} // namespace Graphics
