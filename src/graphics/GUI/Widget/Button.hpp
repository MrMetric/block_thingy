#pragma once
#include "Base.hpp"

#include <functional>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

namespace Graphics {
namespace GUI {

class WidgetContainer;

namespace Widget {

class Button : public Base
{
	public:
		Button
		(
			WidgetContainer& owner,
			const std::string& text,
			std::function<void()> click_handler
		);

		void draw();

		void mousepress(int button, int action, int mods);
		void mousemove(double x, double y);

		glm::dvec4 color;
		glm::dvec4 hover_color;

	private:
		bool hover;
		bool mousedown;
		std::string text;
		glm::dvec2 text_size;
		glm::dvec2 text_position;
		std::function<void()> click_handler;
};

} // Widget
} // GUI
} // Graphics
