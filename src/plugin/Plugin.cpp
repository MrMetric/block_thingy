#include "Plugin.hpp"

#ifdef HAVE_POSIX
	#include <dlfcn.h>
#endif

#include "util/logger.hpp"

using std::string;

namespace block_thingy {

struct Plugin::impl
{
	explicit impl(const fs::path& path);
	~impl();

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	void* get_symbol(const string& name) noexcept;
	void* get_symbol_or_throw(const string& name);

	void* handle;
	fs::path path;
};

Plugin::Plugin
(
	const fs::path& path
)
:
	inited(false),
	name_(path.u8string()),
	pImpl(std::make_unique<impl>(path))
{
	if(pImpl->handle == nullptr)
	{
		pImpl = nullptr;
		return;
	}

	void* get_name_ptr = pImpl->get_symbol("bt_plugin_get_name");
	if(get_name_ptr != nullptr)
	{
		const auto get_name = *reinterpret_cast<string(*)()>(get_name_ptr);
		name_ = get_name();
	}
	else
	{
		LOG(WARN) << "plugin " << name_ << " is missing function get_name\n";
	}
}

Plugin::~Plugin()
{
}

Plugin::Plugin(Plugin&& that) noexcept
:
	pImpl(std::move(that.pImpl))
{
}

void Plugin::init(game& g)
{
	if(pImpl == nullptr)
	{
		return;
	}

	void* init_ptr = pImpl->get_symbol("bt_plugin_init");
	if(init_ptr == nullptr)
	{
		LOG(ERROR) << "plugin " << name_ << " is missing function init\n";
		return;
	}

	const auto init = *reinterpret_cast<void(*)(game&)>(init_ptr);
	init(g);
	inited = true;
	LOG(INFO) << "initialized " << name_ << '\n';
}

void Plugin::load_world(world::world& world)
{
	if(pImpl == nullptr)
	{
		return;
	}

	void* load_world_ptr = pImpl->get_symbol("bt_plugin_load_world");
	if(load_world_ptr == nullptr)
	{
		return;
	}

	const auto load_world = *reinterpret_cast<void(*)(world::world&)>(load_world_ptr);
	load_world(world);
}

Plugin::impl::impl(const fs::path& path)
:
	handle(nullptr),
	path(path)
{
#ifdef HAVE_POSIX
	LOG(INFO) << "opening " << path.u8string() << '\n';
	handle = dlopen(path.c_str(), RTLD_NOW);
	if(handle == nullptr)
	{
		LOG(ERROR) << "unable to open " << path.u8string() << ": " << dlerror() << '\n';
	}
#endif
}

Plugin::impl::~impl()
{
	if(handle == nullptr)
	{
		return;
	}

#ifdef HAVE_POSIX
	if(dlclose(handle) == 0)
	{
		LOG(INFO) << "closed " << path.u8string() << '\n';
	}
	else
	{
		LOG(ERROR) << "error closing " << path.u8string() << ": " << dlerror() << '\n';
	}
#endif
}

void* Plugin::impl::get_symbol(const string& name) noexcept
{
#ifdef HAVE_POSIX
	return dlsym(handle, name.c_str());
#else
	return nullptr;
#endif
}

}
