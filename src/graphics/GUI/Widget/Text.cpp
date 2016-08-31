#include "Text.hpp"

#include <glm/common.hpp>

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/WidgetContainer.hpp"

using std::string;

namespace Graphics {
namespace GUI {
namespace Widget {

Text::Text
(
	WidgetContainer& owner,
	const string& text
)
:
	Base(owner, owner.game.gfx.gui_text.get_size(text), {0.5, 0.5}),
	text(text)
{
}

void Text::draw()
{
	owner.game.gfx.gui_text.draw(text, glm::round(real_position));
}

} // namespace Widget
} // namespace GUI
} // namespace Graphics
