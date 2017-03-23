#include "Plugin.hpp"

#ifdef __linux__
#include <dlfcn.h>
#endif

#include <easylogging++/easylogging++.hpp>

using std::string;

struct Plugin::impl
{
	impl(const string& path)
	:
		path(path)
	{
	}

	string path;
};

Plugin::Plugin
(
	const string& path,
	Game& game
)
:
	pImpl(std::make_unique<impl>(path))
{
#ifdef __linux__
	LOG(INFO) << "loading " << path;
	void* const h = dlopen(path.c_str(), RTLD_NOW);
	if(h == nullptr)
	{
		LOG(ERROR) << dlerror();
		return;
	}
	using init_t = void(*)(Game&);
	const auto init = reinterpret_cast<init_t>(dlsym(h, "init"));
	if(init == nullptr)
	{
		LOG(ERROR) << "error getting symbol 'init' in " << path << ": " << dlerror();
		return;
	}
	(*init)(game);
	handle = h;
	LOG(INFO) << "initialized " << path;
#endif
}

Plugin::~Plugin()
{
	if(pImpl == nullptr)
	{
		return;
	}
	// unloading needs to happen later than it does, so skip it for now
	/*
#ifdef __linux__
	if(handle != nullptr)
	{
		if(dlclose(handle) != 0)
		{
			LOG(WARNING) << "error unloading " << pImpl->path << ": " << dlerror();
		}
		else
		{
			LOG(INFO) << "unloaded " << path;
		}
	}
#endif
	*/
}

Plugin::Plugin(Plugin&& that)
{
	handle = that.handle;
	that.handle = nullptr;

	pImpl = std::move(that.pImpl);
}