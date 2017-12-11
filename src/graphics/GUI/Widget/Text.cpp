#include "Text.hpp"

#include <glm/common.hpp>

#include "Gfx.hpp"

using std::string;

namespace Graphics::GUI::Widget {

Text::Text
(
	const string& text
)
:
	text(text)
{
	size = Gfx::instance->gui_text.get_size(text);
	style["size.x"] = size.x;
	style["size.y"] = size.y;
}

string Text::type() const
{
	return "Text";
}

void Text::draw()
{
	Base::draw();

	Gfx::instance->gui_text.draw(text, position);
}

void Text::read_layout(const json& layout)
{
	if(layout.count("text") != 0)
	{
		text = get_layout_var<string>(layout, "text", "");
		size = Gfx::instance->gui_text.get_size(text);
		style["size.x"] = size.x;
		style["size.y"] = size.y;
	}

	// to avoid overwriting a custom size, call Base *after* text init
	Base::read_layout(layout);
}

}
