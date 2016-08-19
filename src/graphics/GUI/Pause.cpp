#include "Pause.hpp"

#include "Game.hpp"
#include "Gfx.hpp"

namespace Graphics {
namespace GUI {

Pause::Pause(Game& game)
	:
	Base(game)
{
}

void Pause::init()
{
	glfwSetInputMode(game.gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Pause::draw()
{
	parent->draw();
	Base::draw();
}

void Pause::draw_gui()
{
	game.gfx.gui_text.draw("paused", {8, 100});

	game.gfx.draw_rectangle({0.5, 0.5}, {0, 0}, {256, 64}, {0.0, 0.0, 0.0, 0.75});
	game.gfx.draw_rectangle({0.5, 0.5}, {0, 70}, {256, 64}, {0.0, 0.0, 0.0, 0.75});
}

} // namespace GUI
} // namespace Graphics
