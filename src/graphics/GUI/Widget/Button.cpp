#include "Button.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/Base.hpp"
#include "graphics/GUI/WidgetContainer.hpp"

namespace Graphics {
namespace GUI {
namespace Widget {

Button::Button
(
	WidgetContainer& owner,
	const std::string& text,
	std::function<void()> click_handler
)
:
	Base(owner, {256, 64}),
	color(0.01, 0.01, 0.02, 0.85),
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
	const double border_size = 2.0;
	const glm::dvec4 border_color(1.0);
	const glm::dvec2 border_size_x(size.x + border_size * 2, border_size);
	const glm::dvec2 border_size_y(border_size, size.y + border_size * 2);

	const glm::dvec2 border_pos_topleft(real_position - border_size);
	const glm::dvec2 border_pos_bottom(real_position.x - border_size, real_position.y + size.y);
	const glm::dvec2 border_pos_right(real_position.x + size.x, real_position.y - border_size);

	owner.game.gfx.draw_rectangle(border_pos_topleft, border_size_x, border_color);
	owner.game.gfx.draw_rectangle(border_pos_bottom, border_size_x, border_color);
	owner.game.gfx.draw_rectangle(border_pos_topleft, border_size_y, border_color);
	owner.game.gfx.draw_rectangle(border_pos_right, border_size_y, border_color);

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
