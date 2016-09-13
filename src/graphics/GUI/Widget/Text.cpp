#include "Text.hpp"

#include <glm/common.hpp>

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/WidgetContainer.hpp"

using std::string;

namespace Graphics::GUI::Widget {

Text::Text
(
	WidgetContainer& owner,
	const glm::dvec2& origin,
	const string& text
)
:
	Base(owner, owner.game.gfx.gui_text.get_size(text), origin),
	text(text)
{
}

void Text::draw()
{
	Base::draw();

	owner.game.gfx.gui_text.draw(text, glm::round(real_position));
}

} // namespace Graphics::GUI::Widget
