#include "Light.hpp"

#include <stdexcept>
#include <string>

#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"
#include "block/Light.hpp"
#include "graphics/GUI/Widget/TextInput.hpp"

using std::string;

namespace Graphics::GUI {

Light::Light
(
	Game& game,
	World& world,
	Block::Light& block,
	const Position::BlockInWorld& block_pos
)
:
	Base(game, "guis/Light.btgui"),
	world(world),
	block(block),
	block_pos(block_pos)
{
	const Graphics::Color c = block.color();
	for(uint_fast8_t i = 0; i < 3; ++i)
	{
		auto w = root.get_widget_by_id<Widget::TextInput>(std::to_string(i));
		if(w != nullptr)
		{
			w->set_text(std::to_string(c[i]));
			w->on_change([this, i](Widget::TextInput& w, const string& /*old_value*/, const string& new_value)
			{
				on_change(i, w, new_value);
			});
		}
	}
}

string Light::type() const
{
	return "Light";
}

void Light::init()
{
	glfwSetInputMode(game.gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Light::draw()
{
	parent->draw();
	Base::draw();
}

void Light::on_change(uint_fast8_t i, Widget::TextInput& w, const string& new_value)
{
	bool invalid = false;
	int v = 0;
	try
	{
		v = Util::stoi(new_value);
		if(v < 0)
		{
			invalid = true;
			v = 0;
		}
		else if(v > Graphics::Color::max)
		{
			invalid = true;
			v = Graphics::Color::max;
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
	auto c = block.color();
	if(v != c[i])
	{
		c[i] = static_cast<Graphics::Color::value_type>(v);
		block.color(c);
		world.update_blocklight(block_pos, c, true);
	}
}

}
