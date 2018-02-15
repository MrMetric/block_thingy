#pragma once
#include "Base.hpp"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "graphics/GUI/Widget/Component/Text.hpp"
#include "shim/propagate_const.hpp"

namespace block_thingy::graphics::gui::widget {

class text_input : public Base
{
public:
	text_input
	(
		Base* parent,
		const std::string& content = "",
		const std::string& placeholder = ""
	);
	~text_input() override;

	std::string type() const override;

	void draw() override;

	void keypress(const input::key_press&) override;
	void charpress(const input::char_press&) override;
	void mousepress(const input::mouse_press&) override;

	void read_layout(const json&) override;

	std::string get_text() const;
	void set_text(const std::string&, bool reset_cursor = true);
	void clear();
	void delete_selection();

	using on_change_callback_t = std::function<void(text_input&, const std::string& old_value, const std::string& new_value)>;
	void on_change(on_change_callback_t);
	void trigger_on_change(const std::string& old_value);

	using on_keypress_callback_t = std::function<void(text_input&, const input::key_press&)>;
	void on_keypress(on_keypress_callback_t);
	void trigger_on_keypress(const input::key_press&);

	glm::dvec4 color;
	glm::dvec4 color_disabled;
	glm::dvec4 color_invalid;
	const glm::dvec4& get_color() const;

	bool enabled() const;
	void enabled(bool);

	bool valid() const;
	void valid(bool);

	bool focused() const;
	void focused(bool);

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
