#pragma once
#include "Base.hpp"

#include <functional>
#include <string>
#include <vector>

#include "graphics/GUI/Widget/Component/Text.hpp"

namespace Graphics::GUI::Widget {

class TextInput : public Base
{
	public:
		TextInput
		(
			const std::string& content = "",
			const std::string& placeholder = ""
		);

		std::string type() const override;

		void draw() override;

		void keypress(const Util::key_press&) override;
		void charpress(const Util::char_press&) override;
		void mousepress(const Util::mouse_press&) override;

		void read_layout(const json&) override;

		std::string get_text() const;
		void set_text(const std::string&);

		using on_change_callback_t = std::function<void(TextInput&, const std::string&)>;
		void on_change(on_change_callback_t);
		void trigger_on_change();

		using on_keypress_callback_t = std::function<void(TextInput&, const Util::key_press&)>;
		void on_keypress(on_keypress_callback_t);
		void trigger_on_keypress(const Util::key_press&);

		bool invalid;

	private:
		Component::Text content;
		Component::Text placeholder;
		bool focus;
		std::vector<on_change_callback_t> on_change_callbacks;
		std::vector<on_keypress_callback_t> on_keypress_callbacks;
};

}
