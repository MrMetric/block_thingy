#include "Button.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/Base.hpp"
#include "graphics/GUI/WidgetContainer.hpp"

namespace Graphics {
namespace GUI {
namespace Widget {

Button::Button(WidgetContainer& owner, const glm::dvec2& position, const glm::dvec2& offset, const std::string& text)
	:
	Base(owner, position, offset, {256, 64}),
	color(0, 0, 0, 0.8),
	text(text),
	text_size(owner.game.gfx.gui_text.get_size(text)),
	text_position((size - text_size) / 2.0)
{
}

void Button::draw()
{
	owner.game.gfx.draw_rectangle(real_position, size, color);
	owner.game.gfx.gui_text.draw(text, glm::round(real_position + text_position));
}

void Button::mousepress(const int button, const int action, const int mods)
{
	// TODO
}

void Button::mousemove(const double x, const double y)
{
	if(x >= real_position.x && x < real_position.x + size.x
	&& y >= real_position.y && y < real_position.y + size.y)
	{
		color.w = 1.0;
	}
	else
	{
		color.w = 0.8;
	}
}

} // Widget
} // GUI
} // Graphics
