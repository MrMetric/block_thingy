#include "light.hpp"

#include <stdexcept>
#include <string>

#include <GLFW/glfw3.h>

#include "game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/Widget/text_input.hpp"
#include "util/misc.hpp"

using std::nullopt;
using std::string;

namespace block_thingy::graphics::gui {

light::light
(
	game& g,
	world::world& world,
	const position::block_in_world& block_pos,
	const block_t block
)
:
	Base(g, "guis/light.btgui"),
	world(world),
	block_pos(block_pos),
	block(block),
	color(world.block_manager.info.light(block))
{
	for(std::ptrdiff_t i = 0; i < 3; ++i)
	{
		auto w = root.get_widget_by_id_t<widget::text_input>(std::to_string(i));
		if(w != nullptr)
		{
			w->set_text(std::to_string(color[i]));
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

void light::close()
{
	if(color != world.block_manager.info.light(block))
	{
		const string strid = "test_light_"
		                    + std::to_string(color[0]) + '_'
		                    + std::to_string(color[1]) + '_'
		                    + std::to_string(color[2]);
		if(const std::optional<block_t> b = world.block_manager.get_block(strid);
			b != nullopt)
		{
			block = *b;
		}
		else
		{
			block = world.block_manager.duplicate(block);
			world.block_manager.set_strid(block, strid);
			world.block_manager.info.light(block, color);
		}
		world.set_block(block_pos, block);
	}

	Base::close();
}

void light::draw()
{
	parent->draw();
	Base::draw();
}

void light::on_change(std::ptrdiff_t i, widget::text_input& w, const string& new_value)
{
	bool valid = false;
	int v = 0;
	if(const std::optional<int> v_ = util::stoi(new_value);
		v_ != nullopt)
	{
		if(*v_ < 0)
		{
			// v is already 0
		}
		else if(*v_ > graphics::color::max)
		{
			v = graphics::color::max;
		}
		else
		{
			valid = true;
			v = *v_;
		}
	}
	w.valid(valid);
	color[i] = static_cast<graphics::color::value_type>(v);
}

}
