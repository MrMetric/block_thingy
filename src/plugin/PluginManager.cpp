#include "PluginManager.hpp"

#ifdef __linux__
	#include <dlfcn.h>
#endif

#include <vector>

#include "plugin/Plugin.hpp"
#include "util/filesystem.hpp"
#include "util/misc.hpp"

using std::string;

namespace block_thingy {

struct PluginManager::impl
{
	std::vector<Plugin> plugins;

	impl() = default;

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;
};

PluginManager* PluginManager::instance = nullptr;

PluginManager::PluginManager()
:
	set_instance(this),
	pImpl(std::make_unique<impl>())
{
	fs::create_directory("plugins");
	for(const auto& entry : fs::directory_iterator("plugins"))
	{
	#ifdef _WIN32
		const fs::path sopath = entry.path() / "plugin.dll";
	#else
		const fs::path sopath = entry.path() / "plugin.so";
	#endif
		if(!fs::is_regular_file(sopath))
		{
			continue;
		}
		pImpl->plugins.emplace_back(sopath);
	}
}

PluginManager::~PluginManager()
{
}

void PluginManager::plugin_init(game& game)
{
	for(Plugin& plugin : pImpl->plugins)
	{
		plugin.init(game);
	}
}

void PluginManager::plugin_load_world(world::world& world)
{
	for(Plugin& plugin : pImpl->plugins)
	{
		plugin.load_world(world);
	}
}

}
