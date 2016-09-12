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
			WidgetContainer&,
			const std::string& content = "",
			const std::string& placeholder = ""
		);

		void draw() override;

		void keypress(int key, int scancode, int action, Util::key_mods) override;
		void charpress(char32_t, Util::key_mods) override;
		void mousepress(int button, int action, Util::key_mods) override;
		void mousemove(double x, double y) override;

		using on_change_callback_t = std::function<void(TextInput&, const std::string&)>;
		void on_change(on_change_callback_t);
		void trigger_on_change();

		bool invalid;

	private:
		Component::Text content;
		Component::Text placeholder;
		bool hover;
		bool focus;
		std::vector<on_change_callback_t> on_change_callbacks;
};

}
