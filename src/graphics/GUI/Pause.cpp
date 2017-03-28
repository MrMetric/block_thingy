#include "Pause.hpp"

#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "console/Console.hpp"
#include "graphics/GUI/Widget/Button.hpp"
#include "graphics/GUI/Widget/Text.hpp"
#include "graphics/GUI/Widget/TextInput.hpp"
#include "graphics/GUI/Widget/Component/Border.hpp"

using std::string;

namespace Graphics::GUI {

Pause::Pause(Game& game)
:
	Base(game, WidgetContainerMode::widgets)
{
	root.add<Widget::Text>(glm::dvec2(0.5, 0.5), "Paused");

	auto& btn_resume = root.add<Widget::Button>("Resume",
	[&game]()
	{
		game.console.run_line("close_gui");
	});

	auto& btn_save = root.add<Widget::Button>("Save",
	[&game]()
	{
		game.console.run_line("save");
	});

	auto& btn_save_quit = root.add<Widget::Button>("Save & Quit",
	[&game]()
	{
		game.console.run_line("save");
		game.console.run_line("quit");
	});

	auto& btn_quit = root.add<Widget::Button>("Quit",
	[&game]()
	{
		game.console.run_line("quit");
	});

	auto border = std::make_shared<Widget::Component::Border>(2, glm::dvec4(1));
	btn_resume.add_modifier(border);
	btn_save.add_modifier(border);
	btn_save_quit.add_modifier(border);
	btn_quit.add_modifier(border);
}

string Pause::type() const
{
	return "pause";
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

} // namespace Graphics::GUI
