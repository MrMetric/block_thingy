#pragma once
#include "Base.hpp"

#include <string>

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

		void keypress(int key, int scancode, int action, int mods) override;
		void charpress(char32_t codepoint) override;
		void mousepress(int button, int action, int mods) override;
		void mousemove(double x, double y) override;

	private:
		Component::Text content;
		Component::Text placeholder;
		bool hover;
		bool focus;
};

}
