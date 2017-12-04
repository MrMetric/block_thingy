#pragma once

#include <memory>

#include "fwd/Game.hpp"
#include "shim/propagate_const.hpp"
#include "util/filesystem.hpp"

class Plugin
{
public:
	Plugin
	(
		const fs::path&
	);
	~Plugin();

	Plugin(Plugin&&);
	Plugin(const Plugin&) = delete;
	void operator=(const Plugin&) = delete;

private:
	friend class PluginManager;

	void init(Game&);

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};
