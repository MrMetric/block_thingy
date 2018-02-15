#include "Text.hpp"

#include "Gfx.hpp"
#include "util/unicode.hpp"

using std::string;
using std::u32string;

namespace block_thingy::graphics::gui::widget::component {

Text::Text(const string& s)
{
	operator=(s);
}

void Text::draw(const glm::dvec2& w_position, const glm::dvec2& /*w_size*/) const
{
	Gfx::instance->gui_text.draw(text, w_position);
}

string Text::get8() const
{
	return util::utf32_to_utf8(text);
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

Text& Text::operator=(const char c)
{
	return operator=(util::utf8_to_utf32(c));
}
Text& Text::operator=(const char32_t c)
{
	text = c;
	update_info();
	return *this;
}
Text& Text::operator=(const string& s)
{
	return operator=(util::utf8_to_utf32(s));
}
Text& Text::operator=(const u32string& s)
{
	text = s;
	update_info();
	return *this;
}

Text& Text::operator+=(const char c)
{
	return operator+=(util::utf8_to_utf32(c));
}
Text& Text::operator+=(const char32_t c)
{
	text += c;
	update_info();
	return *this;
}
Text& Text::operator+=(const string& s)
{
	return operator+=(util::utf8_to_utf32(s));
}
Text& Text::operator+=(const u32string& s)
{
	text += s;
	update_info();
	return *this;
}

bool Text::operator==(const char c) const
{
	return operator==(util::utf8_to_utf32(c));
}
bool Text::operator==(const char32_t c) const
{
	return text.size() == 1 && text[0] == c;
}
bool Text::operator==(const string& s) const
{
	return text == util::utf8_to_utf32(s);
}
bool Text::operator==(const u32string& s) const
{
	return text == s;
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
