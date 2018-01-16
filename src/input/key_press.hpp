#pragma once

#include "input/key_mods.hpp"
#include "fwd/input/mouse_press.hpp"

namespace block_thingy::util {

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
