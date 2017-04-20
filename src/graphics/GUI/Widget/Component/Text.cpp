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
	// TODO
}

string Text::get8() const
{
	return Util::utf32_to_utf8(text);
}
u32string Text::get32() const
{
	return text;
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
