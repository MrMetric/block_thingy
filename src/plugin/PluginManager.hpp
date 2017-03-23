#pragma once

#include <memory>
#include <experimental/propagate_const>

#include "fwd/Game.hpp"

class PluginManager
{
public:
	PluginManager();
	~PluginManager();

	PluginManager(PluginManager&&) = delete;
	PluginManager(const PluginManager&) = delete;
	void operator=(const PluginManager&) = delete;

	void init_plugins(Game&);

private:
	struct impl;
	std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};
