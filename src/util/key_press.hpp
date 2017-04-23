#pragma once

#include "util/key_mods.hpp"
#include "fwd/util/mouse_press.hpp"

namespace Util {

struct key_press
{
	key_press(int key, int scancode, int action, int mods);
	key_press(int key, int scancode, int action, key_mods mods);

	int key;
	int scancode;
	int action;
	key_mods mods;
};

}
