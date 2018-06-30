#pragma once

#include <memory>

#include "fwd/game.hpp"
#include "shim/propagate_const.hpp"
#include "util/filesystem.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy {

class Plugin
{
public:
	Plugin
	(
		const fs::path&
	);
	~Plugin();

	Plugin(Plugin&&) noexcept;
	Plugin(const Plugin&) = delete;
	Plugin& operator=(Plugin&&) = delete;
	Plugin& operator=(const Plugin&) = delete;

	bool is_inited() const
	{
		return inited;
	}
	std::string name() const
	{
		return name_;
	}

private:
	friend class PluginManager;

	bool inited;
	std::string name_;

	void init(game&);
	void load_world(world::world&);

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
