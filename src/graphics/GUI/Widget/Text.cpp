#include "Text.hpp"

#include <glm/common.hpp>

#include "Gfx.hpp"

using std::string;

namespace block_thingy::graphics::gui::widget {

struct Text::impl
{
	impl(const string& text)
	:
		text(text)
	{
	}

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	component::Text text;
};

Text::Text
(
	Base* const parent,
	const string& text
)
:
	Base(parent),
	pImpl(std::make_unique<impl>(text))
{
	size = pImpl->text.get_size();
	style["size.x"] = size.x;
	style["size.y"] = size.y;
}

Text::~Text()
{
}

string Text::type() const
{
	return "text";
}

void Text::draw()
{
	Base::draw();

	pImpl->text.draw(position, size);
}

void Text::read_layout(const json& layout)
{
	set_text(get_layout_var<string>(layout, "text", ""));

	// to avoid overwriting a custom size, call Base *after* text init
	Base::read_layout(layout);
}

string Text::get_text() const
{
	return pImpl->text.get8();
}

void Text::set_text(const string& s)
{
	auto& text = pImpl->text;

	if(text == s)
	{
		return;
	}

	text = s;
	size = text.get_size();
	style["size.x"] = size.x;
	style["size.y"] = size.y;
}

}
