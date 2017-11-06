#include "key_mods.hpp"

#include <GLFW/glfw3.h>

namespace Util {

key_mods::key_mods()
:
	key_mods(0)
{
}

key_mods::key_mods(const int mods)
:
	mods(mods)
{
}

bool key_mods::is(const bool shift, const bool ctrl, const bool alt, const bool super) const
{
	return (shift == this->shift())
		&& (ctrl == this->ctrl())
		&& (alt == this->alt())
		&& (super == this->super())
		;
}

bool key_mods::none() const
{
	return is(false, false, false, false);
}

bool key_mods::shift() const
{
	return mods & GLFW_MOD_SHIFT;
}

bool key_mods::ctrl() const
{
	return mods & GLFW_MOD_CONTROL;
}

bool key_mods::alt() const
{
	return mods & GLFW_MOD_ALT;
}

bool key_mods::super() const
{
	return mods & GLFW_MOD_SUPER;
}

bool key_mods::shift_only() const
{
	return mods == GLFW_MOD_SHIFT;
}

bool key_mods::ctrl_only() const
{
	return mods == GLFW_MOD_CONTROL;
}

bool key_mods::alt_only() const
{
	return mods == GLFW_MOD_ALT;
}

bool key_mods::super_only() const
{
	return mods == GLFW_MOD_SUPER;
}

}
