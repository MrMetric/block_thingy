#include "Plugin.hpp"

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
	#define HAVE_POSIX
#endif


#ifdef HAVE_POSIX
	#include <dlfcn.h>
#endif

#include <easylogging++/easylogging++.hpp>

#include "Game.hpp"

using std::string;

struct Plugin::impl
{
	impl(const string& path)
	:
		path(path)
	{
	#ifdef HAVE_POSIX
		LOG(INFO) << "loading " << path;
		handle = dlopen(path.c_str(), RTLD_NOW);
		if(handle == nullptr)
		{
			LOG(ERROR) << dlerror();
			LOG(ERROR) << "unable to load " << path;
		}
	#endif
	}

	~impl()
	{
	#ifdef HAVE_POSIX
		if(handle != nullptr)
		{
			if(dlclose(handle) == 0)
			{
				LOG(INFO) << "unloaded " << path;
			}
			else
			{
				LOG(WARNING) << "error unloading " << path << ": " << dlerror();
			}
		}
	#endif
	}

	void* get_symbol(const string& name)
	{
	#ifdef HAVE_POSIX
		void* symbol = dlsym(handle, name.c_str());
		if(symbol == nullptr)
		{
			throw std::runtime_error("Error getting symbol '" + name + "' in " + path + ": " + dlerror());
		}
		return symbol;
	#endif
	}

	void* handle;
	string path;
};

Plugin::Plugin
(
	const string& path
)
:
	pImpl(std::make_unique<impl>(path))
{
	if(pImpl->handle == nullptr)
	{
		pImpl = nullptr;
	}
}

Plugin::~Plugin()
{
}

Plugin::Plugin(Plugin&& that)
{
	pImpl = std::move(that.pImpl);
}

void Plugin::init()
{
	if(pImpl == nullptr) return;

#ifdef HAVE_POSIX
	using init_t = void(*)(Game&);
	const auto init = *reinterpret_cast<init_t>(pImpl->get_symbol("init"));
	init(*Game::instance);
	LOG(INFO) << "initialized " << pImpl->path;
#endif
}
