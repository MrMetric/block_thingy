#include "main_menu.hpp"

#include <glad/glad.h>

#include "game.hpp"
#include "graphics/GUI/Widget/Button.hpp"

using std::string;

namespace block_thingy::graphics::gui {

main_menu::main_menu(game& g)
:
	Base(g, "guis/main_menu.btgui")
{
	auto btn = root.get_widget_by_id_t<widget::Button>("btn_singleplayer");
	if(btn != nullptr)
	{
		btn->on_click([&g](widget::Button&, const glm::dvec2& /*position*/)
		{
			g.load_world("test");
		});
	}
}

string main_menu::type() const
{
	return "main_menu";
}

void main_menu::switch_to()
{
	Base::switch_to();

	glClearColor(0, 0, 0, 1);
}

}
