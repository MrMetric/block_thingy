#pragma once

#include <cassert>
#include <memory>

#include "fwd/game.hpp"
#include "shim/propagate_const.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy {

class PluginManager
{
public:
	PluginManager();
	~PluginManager();

	PluginManager(PluginManager&&) = delete;
	PluginManager(const PluginManager&) = delete;
	PluginManager& operator=(PluginManager&&) = delete;
	PluginManager& operator=(const PluginManager&) = delete;

	void plugin_init(game&);
	void plugin_load_world(world::world&);

	static PluginManager* instance;

private:
	class set_instance
	{
		public:
			set_instance(PluginManager* ptr)
			{
				assert(PluginManager::instance == nullptr);
				PluginManager::instance = ptr;
			}
	} set_instance;

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
