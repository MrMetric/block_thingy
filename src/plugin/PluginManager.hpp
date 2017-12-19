#pragma once

#include <cassert>
#include <memory>

#include "fwd/game.hpp"
#include "shim/propagate_const.hpp"

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

	void init_plugins(game&);

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
