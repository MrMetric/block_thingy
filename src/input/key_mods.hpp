#pragma once

namespace block_thingy::util {

struct key_mods
{
	key_mods();
	key_mods(int mods);

	bool is(bool shift, bool ctrl, bool alt, bool super) const;
	bool none() const;
	bool shift() const;
	bool ctrl() const;
	bool alt() const;
	bool super() const;
	bool shift_only() const;
	bool ctrl_only() const;
	bool alt_only() const;
	bool super_only() const;

	const int mods;
};

}
