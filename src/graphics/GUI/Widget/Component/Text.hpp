#pragma once
#include "Base.hpp"

#include <string>

#include <glm/vec2.hpp>

namespace Graphics::GUI::Widget::Component {

class Text : public Base
{
public:
	Text(const std::string&);

	void draw(const glm::dvec2& w_position, const glm::dvec2& w_size) override;
	glm::dvec2 draw_position(const glm::dvec2& w_position, const glm::dvec2& w_size) const;

	std::string get8() const;
	std::u32string get32() const;
	bool empty() const;
	void clear();

	Text& operator=(char32_t);
	Text& operator=(const std::string&);
	Text& operator=(const std::u32string&);

	Text& operator+=(const char32_t);
	Text& operator+=(const std::string&);
	Text& operator+=(const std::u32string&);

	void pop_back();
	std::size_t str_size() const;

	glm::dvec2 get_size() const;

private:
	std::u32string text;
	glm::dvec2 size;

	void update_info();
};

}
