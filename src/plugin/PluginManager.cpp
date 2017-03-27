#include "PluginManager.hpp"

#ifdef __linux__
#include <dlfcn.h>
#endif

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif

#include <vector>

#include <easylogging++/easylogging++.hpp>

#include "Plugin.hpp"
#include "Util.hpp"

using std::string;

struct PluginManager::impl
{
	std::vector<Plugin> plugins;
};

PluginManager::PluginManager()
:
	pImpl(std::make_unique<impl>())
{
	fs::create_directory("plugins");
}

PluginManager::~PluginManager()
{
}

void PluginManager::init_plugins(Game& game)
{
	for(const auto& dir : fs::directory_iterator("plugins"))
	{
		const fs::path sopath = dir.path() / "plugin.so";
		if(!fs::is_regular_file(sopath))
		{
			continue;
		}
		pImpl->plugins.emplace_back(sopath, game);
	}
}
