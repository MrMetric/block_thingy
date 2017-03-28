#include "Light.hpp"

#include <stdexcept>
#include <string>

#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"
#include "block/Light.hpp"
#include "graphics/GUI/Widget/TextInput.hpp"
#include "graphics/GUI/Widget/Component/Border.hpp"

using std::string;

namespace Graphics::GUI {

Light::Light
(
	Game& game,
	Block::Light& block
)
:
	Base(game, WidgetContainerMode::widgets),
	block(block)
{
	auto c = block.color();

	auto& r = root.add<Widget::TextInput>(std::to_string(c.r), "red");
	r.on_change([this](Widget::TextInput& w, const string& new_value)
	{
		on_change(0, w, new_value);
	});

	auto& g = root.add<Widget::TextInput>(std::to_string(c.g), "green");
	g.on_change([this](Widget::TextInput& w, const string& new_value)
	{
		on_change(1, w, new_value);
	});

	auto& b = root.add<Widget::TextInput>(std::to_string(c.b), "blue");
	b.on_change([this](Widget::TextInput& w, const string& new_value)
	{
		on_change(2, w, new_value);
	});

	// TODO: theme stuff
	auto border = std::make_shared<Widget::Component::Border>(2, glm::dvec4(1));
	r.add_modifier(border);
	g.add_modifier(border);
	b.add_modifier(border);
}

string Light::type() const
{
	return "light";
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
	}
}

}
