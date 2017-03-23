#pragma once

#include <memory>
#include <experimental/propagate_const>
#include <string>

#include "fwd/Game.hpp"

class Plugin
{
public:
	Plugin
	(
		const std::string& path,
		Game&
	);
	~Plugin();

	Plugin(Plugin&&);
	Plugin(const Plugin&) = delete;
	void operator=(const Plugin&) = delete;

private:
	friend class PluginManager;

	void* handle;
	struct impl;
	std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};
