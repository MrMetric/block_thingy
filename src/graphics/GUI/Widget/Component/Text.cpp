#include "Text.hpp"

#include "Gfx.hpp"
#include "util/unicode.hpp"

using std::string;
using std::u32string;

namespace Graphics::GUI::Widget::Component {

Text::Text(const string& s)
{
	operator=(s);
}

void Text::draw(const glm::dvec2& w_position, const glm::dvec2& w_size)
{
	Gfx::instance->gui_text.draw(text, draw_position(w_position, w_size));
}

glm::dvec2 Text::draw_position(const glm::dvec2& w_position, const glm::dvec2& w_size) const
{
	return glm::round(glm::dvec2
	{
		w_position.x + 8,
		w_position.y + (w_size.y - size.y) * 0.5,
	});
}

string Text::get8() const
{
	return Util::utf32_to_utf8(text);
}
u32string Text::get32() const
{
	return text;
}

bool Text::empty() const
{
	return text.empty();
}

void Text::clear()
{
	operator=(u32string());
}

Text& Text::operator=(const char32_t c)
{
	text = c;
	update_info();
	return *this;
}
Text& Text::operator=(const string& s)
{
	return operator=(Util::utf8_to_utf32(s));
}
Text& Text::operator=(const u32string& s)
{
	text = s;
	update_info();
	return *this;
}

Text& Text::operator+=(const char32_t c)
{
	text += c;
	update_info();
	return *this;
}
Text& Text::operator+=(const string& s)
{
	return operator+=(Util::utf8_to_utf32(s));
}
Text& Text::operator+=(const u32string& s)
{
	text += s;
	update_info();
	return *this;
}

void Text::pop_back()
{
	text.pop_back();
	update_info();
}

std::size_t Text::str_size() const
{
	return text.size();
}

glm::dvec2 Text::get_size() const
{
	return size;
}

void Text::update_info()
{
	size = Gfx::instance->gui_text.get_size(text);
}

}
