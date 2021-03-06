#include "Console.hpp"

#include <GLFW/glfw3.h>

#include "Gfx.hpp"
#include "console/Console.hpp"
#include "graphics/GUI/Widget/text_input.hpp"
#include "input/key_press.hpp"

using std::string;

namespace block_thingy::graphics::gui {

Console::Console(game& g)
:
	Base(g, "guis/console.btgui")
{
	auto input = root.get_widget_by_id_t<widget::text_input>("input");
	if(input != nullptr)
	{
		input->on_keypress([](widget::text_input& input, const input::key_press& press)
		{
			if(press.key == GLFW_KEY_ENTER)
			{
				::block_thingy::Console::instance->run_line(input.get_text());
				input.clear();
			}
		});
		input->focused(true);
	}
}

string Console::type() const
{
	return "console";
}

void Console::draw()
{
	parent->draw();
	Base::draw();
}

}
