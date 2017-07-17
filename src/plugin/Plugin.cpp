#include "Plugin.hpp"

#ifdef HAVE_POSIX
	#include <dlfcn.h>
#endif

#include <easylogging++/easylogging++.hpp>

#include "Game.hpp"

using std::string;

struct Plugin::impl
{
	impl(const fs::path& path)
	:
		path(path)
	{
	#ifdef HAVE_POSIX
		LOG(INFO) << "loading " << path.u8string();
		handle = dlopen(path.c_str(), RTLD_NOW);
		if(handle == nullptr)
		{
			LOG(ERROR) << dlerror();
			LOG(ERROR) << "unable to load " << path.u8string();
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
				LOG(INFO) << "unloaded " << path.u8string();
			}
			else
			{
				LOG(WARNING) << "error unloading " << path.u8string() << ": " << dlerror();
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
			throw std::runtime_error("Error getting symbol '" + name + "' in " + path.u8string() + ": " + dlerror());
		}
		return symbol;
	#else
		return nullptr;
	#endif
	}

	void* handle;
	fs::path path;
};

Plugin::Plugin
(
	const fs::path& path
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
	LOG(INFO) << "initialized " << pImpl->path.u8string();
#endif
}