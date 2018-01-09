#pragma once

#include <memory>

#include "fwd/game.hpp"
#include "shim/propagate_const.hpp"
#include "util/filesystem.hpp"

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

private:
	friend class PluginManager;

	void init(game&);

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
