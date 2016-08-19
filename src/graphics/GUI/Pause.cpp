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
}

void Pause::update_framebuffer_size(const window_size_t& window_size)
{
}

} // namespace GUI
} // namespace Graphics
