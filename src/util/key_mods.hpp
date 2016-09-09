#pragma once

namespace Util {

struct key_mods
{
	explicit key_mods(int mods);

	bool is(bool shift, bool ctrl, bool alt, bool super) const;
	bool none() const;
	bool shift() const;
	bool ctrl() const;
	bool alt() const;
	bool super() const;

	const int mods;
};

}
