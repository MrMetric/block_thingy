#include "Pause.hpp"

#include <GLFW/glfw3.h>

#include "game.hpp"
#include "Gfx.hpp"
#include "console/Console.hpp"
#include "graphics/GUI/Widget/Button.hpp"
#include "graphics/GUI/Widget/Text.hpp"
#include "graphics/GUI/Widget/text_input.hpp"
#include "graphics/opengl/push_state.hpp"

using std::string;

namespace block_thingy::graphics::gui {

Pause::Pause(game& g)
:
	Base(g, "guis/pause.btgui")
{
}

string Pause::type() const
{
	return "pause";
}

void Pause::init()
{
	glfwSetInputMode(g.gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Pause::draw()
{
	parent->draw();

	{
		opengl::push_state<GLboolean, GL_DEPTH_TEST> _depth_test(false);
		Gfx::instance->draw_rectangle({0, 0}, static_cast<glm::dvec2>(Gfx::instance->window_size), {0, 0, 0, 0.3});
	}

	Base::draw();
}

}
