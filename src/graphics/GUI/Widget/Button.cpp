#include "Button.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/Base.hpp"
#include "graphics/GUI/WidgetContainer.hpp"

namespace Graphics {
namespace GUI {
namespace Widget {

Button::Button(
			WidgetContainer& owner,
			const glm::dvec2& position,
			const glm::dvec2& offset,
			const std::string& text,
			std::function<void()> click_handler
	)
	:
	Base(owner, position, offset, {256, 64}),
	color(0, 0, 0, 0.8),
	hover_color(0, 0, 0, 1),
	hover(false),
	mousedown(false),
	text(text),
	text_size(owner.game.gfx.gui_text.get_size(text)),
	text_position((size - text_size) / 2.0),
	click_handler(click_handler)
{
}

void Button::draw()
{
	owner.game.gfx.draw_rectangle(real_position, size, hover ? hover_color : color);
	owner.game.gfx.gui_text.draw(text, glm::round(real_position + text_position));
}

void Button::mousepress(const int button, const int action, const int mods)
{
	if(!hover)
	{
		if(mousedown && action == GLFW_RELEASE)
		{
			// click cancel
			mousedown = false;
		}
	}
	else if(action == GLFW_PRESS)
	{
		// mouse down
		mousedown = true;
	}
	else if(mousedown && action == GLFW_RELEASE)
	{
		// mouse up (click)
		mousedown = false;
		click_handler();
	}
}

void Button::mousemove(const double x, const double y)
{
	hover = x >= real_position.x && x < real_position.x + size.x
		 && y >= real_position.y && y < real_position.y + size.y;
}

} // Widget
} // GUI
} // Graphics
