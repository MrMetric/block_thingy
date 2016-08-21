#include "Pause.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/Widget/Button.hpp"

namespace Graphics {
namespace GUI {

Pause::Pause(Game& game)
	:
	Base(game, WidgetContainerMode::widgets)
{
	root.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, -105}, "Resume",
	[&game]()
	{
		game.console.run_line("close_gui");
	});

	root.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, -35}, "Save",
	[&game]()
	{
		game.console.run_line("save");
	});

	root.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, 35}, "Save & Quit",
	[&game]()
	{
		game.console.run_line("save");
		game.console.run_line("quit");
	});

	root.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, 105}, "Quit",
	[&game]()
	{
		game.console.run_line("quit");
	});
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
	game.gfx.gui_text.draw("paused", {8, 150});
}

} // namespace GUI
} // namespace Graphics
