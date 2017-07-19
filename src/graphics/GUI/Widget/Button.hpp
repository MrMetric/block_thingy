#pragma once
#include "Base.hpp"

#include <functional>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace Graphics::GUI::Widget {

class Button : public Base
{
public:
	Button
	(
		const std::string& text = ""
	);

	std::string type() const override;

	void draw() override;

	void mousepress(const Util::mouse_press&) override;

	void read_layout(const json&) override;
	void use_layout() override;

	void set_text(const std::string&);

	void on_click(std::function<void()> click_handler);

	glm::dvec4 color;
	glm::dvec4 hover_color;

private:
	bool mousedown;
	std::string text;
	glm::dvec2 text_size;
	glm::dvec2 text_position;
	std::vector<std::function<void()>> click_handlers;
};

}
