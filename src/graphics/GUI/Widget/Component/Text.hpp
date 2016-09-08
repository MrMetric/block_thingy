#pragma once
#include "Base.hpp"

#include <string>

#include <glm/vec2.hpp>

namespace Graphics::GUI::Widget::Component {

class Text : public Base
{
	public:
		Text(const std::string&);

		void draw(const Widget::Base&) override;

		//std::string get8() const;
		std::u32string get32() const;
		Text& operator=(const std::string& utf8);
		Text& operator=(const std::u32string& utf32);
		Text& operator+=(const char32_t);
		void pop_back();

		glm::dvec2 get_size() const;

	private:
		std::u32string text;
		glm::dvec2 size;

		void update_info();
};

} // namespace Graphics::GUI::Widget::Component
