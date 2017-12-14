#include "Console.hpp"

#include <GLFW/glfw3.h>

#include "Gfx.hpp"
#include "console/Console.hpp"
#include "graphics/GUI/Widget/TextInput.hpp"
#include "util/key_press.hpp"

using std::string;

namespace block_thingy::graphics::gui {

Console::Console(Game& game)
:
	Base(game, "guis/Console.btgui")
{
	auto input = root.get_widget_by_id<widget::TextInput>("input");
	if(input != nullptr)
	{
		input->on_keypress([](widget::TextInput& input, const util::key_press& press)
		{
			if(press.key == GLFW_KEY_ENTER)
			{
				::block_thingy::Console::instance->run_line(input.get_text());
				input.clear();
			}
		});
		input->set_focus(true);
	}
}

string Console::type() const
{
	return "Console";
}

void Console::init()
{
	glfwSetInputMode(Gfx::instance->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Console::draw()
{
	parent->draw();
	Base::draw();
}

}
