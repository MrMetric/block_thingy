#pragma once

#include "util/key_mods.hpp"

namespace block_thingy::util {

struct mouse_press
{
	mouse_press(int button, int action, int mods);
	mouse_press(int button, int action, key_mods mods);

	int button;
	int action;
	key_mods mods;
};

}
