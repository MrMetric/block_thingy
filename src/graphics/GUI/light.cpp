#include "light.hpp"

#include <stdexcept>
#include <string>

#include <GLFW/glfw3.h>

#include "game.hpp"
#include "Gfx.hpp"
#include "block/test_light.hpp"
#include "graphics/GUI/Widget/text_input.hpp"
#include "util/misc.hpp"

using std::string;

namespace block_thingy::graphics::gui {

light::light
(
	game& g,
	world::world& world,
	block::test_light& block,
	const position::block_in_world& block_pos
)
:
	Base(g, "guis/light.btgui"),
	world(world),
	block(block),
	block_pos(block_pos)
{
	const graphics::color c = block.light();
	for(std::ptrdiff_t i = 0; i < 3; ++i)
	{
		auto w = root.get_widget_by_id<widget::text_input>(std::to_string(i));
		if(w != nullptr)
		{
			w->set_text(std::to_string(c[i]));
			w->on_change([this, i](widget::text_input& w, const string& /*old_value*/, const string& new_value)
			{
				on_change(i, w, new_value);
			});
		}
	}
}

string light::type() const
{
	return "light";
}

void light::draw()
{
	parent->draw();
	Base::draw();
}

void light::on_change(std::ptrdiff_t i, widget::text_input& w, const string& new_value)
{
	bool invalid = false;
	int v = 0;
	try
	{
		v = util::stoi(new_value);
		if(v < 0)
		{
			invalid = true;
			v = 0;
		}
		else if(v > graphics::color::max)
		{
			invalid = true;
			v = graphics::color::max;
		}
	}
	catch(const std::invalid_argument&)
	{
		invalid = true;
	}
	catch(const std::out_of_range&)
	{
		invalid = true;
	}
	w.invalid = invalid;
	auto c = block.light();
	if(v != c[i])
	{
		c[i] = static_cast<graphics::color::value_type>(v);
		block.light(c);
		world.update_blocklight(block_pos, c, true);
	}
}

}
