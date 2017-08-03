#pragma once

#include <cassert>
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
	std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};
