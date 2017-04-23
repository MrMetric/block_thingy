#include "key_press.hpp"

#include "util/mouse_press.hpp"

namespace Util {

key_press::key_press(const int key, const int scancode, const int action, const int mods)
:
	key_press(key, scancode, action, key_mods(mods))
{
}

key_press::key_press(const int key, const int scancode, const int action, const key_mods mods)
:
	key(key),
	scancode(scancode),
	action(action),
	mods(mods)
{
}

}
