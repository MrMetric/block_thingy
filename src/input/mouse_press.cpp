#include "mouse_press.hpp"

namespace block_thingy::input {

mouse_press::mouse_press(const glm::dvec2& position, const int button, const int action, const int mods)
:
	mouse_press(position, button, action, key_mods(mods))
{
}

mouse_press::mouse_press(const glm::dvec2& position, const int button, const int action, const key_mods mods)
:
	position(position),
	button(button),
	action(action),
	mods(mods)
{
}

}
