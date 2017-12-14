#include "PluginManager.hpp"

#ifdef __linux__
	#include <dlfcn.h>
#endif

#include <vector>

#include "Plugin.hpp"
#include "Util.hpp"
#include "util/filesystem.hpp"

using std::string;

namespace block_thingy {

struct PluginManager::impl
{
	std::vector<Plugin> plugins;
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
		const fs::path sopath = entry.path() / "plugin.so";
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

void PluginManager::init_plugins(Game& game)
{
	for(Plugin& plugin : pImpl->plugins)
	{
		plugin.init(game);
	}
}

}
