#pragma once

#include <memory>
#include <experimental/propagate_const>

#include "fwd/Game.hpp"
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

	void init();

	struct impl;
	std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};
