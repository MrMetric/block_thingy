#pragma once

#include "input/key_mods.hpp"

namespace block_thingy::input {

struct char_press
{
	char_press(char32_t codepoint, int mods);
	char_press(char32_t codepoint, key_mods);

	char32_t codepoint;
	key_mods mods;
};

}
