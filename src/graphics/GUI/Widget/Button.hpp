#pragma once
#include "Base.hpp"

#include <functional>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace block_thingy::graphics::gui::widget {

class Button : public Base
{
public:
	Button
	(
		Base* parent,
		const std::string& text = ""
	);

	std::string type() const override;

	void draw() override;

	void mousepress(const util::mouse_press&) override;

	void read_layout(const json&) override;
	void use_layout() override;

	void set_text(const std::string&);

	void on_click(std::function<void()> click_handler);

	glm::dvec4 color;
	glm::dvec4 color_disabled;
	glm::dvec4 color_hover;
	const glm::dvec4& get_color() const;

	bool get_enabled() const
	{
		return enabled;
	}
	void set_enabled(const bool e)
	{
		enabled = e;
		if(!e)
		{
			mousedown = false;
		}
	}

private:
	bool enabled;
	bool mousedown;
	std::string text;
	glm::dvec2 text_size;
	glm::dvec2 text_position;
	std::vector<std::function<void()>> click_handlers;
};

}
