#pragma once
#include "Base.hpp"

#include <functional>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "shim/propagate_const.hpp"

namespace block_thingy::graphics::gui::widget {

class Button : public Base
{
public:
	Button
	(
		Base* parent,
		const std::string& text = ""
	);
	~Button() override;

	std::string type() const override;

	void draw() override;

	void mousepress(const input::mouse_press&) override;

	void read_layout(const json&) override;
	void use_layout() override;

	void set_text(const std::string&);

	using on_click_callback_t = std::function<void(Button&, const glm::dvec2&)>;
	void on_click(on_click_callback_t);

	glm::dvec4 color;
	glm::dvec4 color_disabled;
	glm::dvec4 color_hover;
	const glm::dvec4& get_color() const;

	bool enabled() const;
	void enabled(bool);

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
