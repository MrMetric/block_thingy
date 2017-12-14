#include "mouse_press.hpp"

namespace block_thingy::util {

mouse_press::mouse_press(const int button, const int action, const int mods)
:
	mouse_press(button, action, key_mods(mods))
{
}

mouse_press::mouse_press(const int button, const int action, const key_mods mods)
:
	button(button),
	action(action),
	mods(mods)
{
}

}
