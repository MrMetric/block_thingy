#pragma once
#include "Base.hpp"

#include <cstddef>
#include <string>

#include <glm/vec2.hpp>

namespace block_thingy::graphics::gui::widget::component {

class Text : public Base
{
public:
	Text(const std::string&);

	void draw(const glm::dvec2& w_position, const glm::dvec2& w_size) const override;

	std::string get8() const;
	std::u32string get32() const;
	bool empty() const;
	void clear();

	Text& operator=(char);
	Text& operator=(char32_t);
	Text& operator=(const std::string&);
	Text& operator=(const std::u32string&);

	Text& operator+=(char);
	Text& operator+=(char32_t);
	Text& operator+=(const std::string&);
	Text& operator+=(const std::u32string&);

	bool operator==(char) const;
	bool operator==(char32_t) const;
	bool operator==(const std::string&) const;
	bool operator==(const std::u32string&) const;

	bool operator!=(char c                 ) const { return !operator==(c); }
	bool operator!=(char32_t c             ) const { return !operator==(c); }
	bool operator!=(const std::string& s   ) const { return !operator==(s); }
	bool operator!=(const std::u32string& s) const { return !operator==(s); }

	void pop_back();
	std::size_t str_size() const;

	glm::dvec2 get_size() const;

private:
	std::u32string text;
	glm::dvec2 size;

	void update_info();
};

}
