#pragma once
#include "Base.hpp"

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
		Button(WidgetContainer& owner, const glm::dvec2& position, const glm::dvec2& offset, const std::string& text);

		void draw();

		void mousepress(int button, int action, int mods);
		void mousemove(double x, double y);

		glm::dvec4 color;

	private:
		std::string text;
		glm::dvec2 text_size;
		glm::dvec2 text_position;
};

} // Widget
} // GUI
} // Graphics
