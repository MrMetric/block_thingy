#include "Pause.hpp"

#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "console/Console.hpp"
#include "graphics/GUI/Widget/Button.hpp"
#include "graphics/GUI/Widget/Text.hpp"
#include "graphics/GUI/Widget/TextInput.hpp"
#include "graphics/GUI/Widget/Component/Border.hpp"

namespace Graphics::GUI {

Pause::Pause(Game& game)
	:
	Base(game, WidgetContainerMode::widgets)
{
	auto& test = root.add<Widget::TextInput>("aaa", "test");

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

	auto button_border = std::make_shared<Widget::Component::Border>(2, glm::dvec4(1));
	btn_resume.add_modifier(button_border);
	btn_save.add_modifier(button_border);
	btn_save_quit.add_modifier(button_border);
	btn_quit.add_modifier(button_border);
	test.add_modifier(button_border);
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
