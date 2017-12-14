#include "char_press.hpp"

#include "util/key_mods.hpp"

namespace block_thingy::util {

char_press::char_press(const char32_t codepoint, const int mods)
:
	char_press(codepoint, key_mods(mods))
{
}

char_press::char_press(const char32_t codepoint, const key_mods mods)
:
	codepoint(codepoint),
	mods(mods)
{
}

}
