#include "FileWatcher.hpp"

#include <vector>

#ifdef USE_INOTIFY
	#include <inotify-cxx/inotify-cxx.hpp>
#endif

using std::string;

namespace block_thingy::util {

struct FileWatcher::impl
{
#ifdef USE_INOTIFY
	Inotify inotify;
	// unique_ptr to keep validity when vector resizes itself
	std::vector<std::unique_ptr<InotifyWatch>> inotify_watches;
#endif

	~impl()
	{
	#ifdef USE_INOTIFY
		// ~Inotify does this, but if inotify_watches destructs first, RemoveAll uses invalid pointers
		inotify.RemoveAll();
	#endif
	}
};

FileWatcher::FileWatcher()
:
	pImpl(std::make_unique<impl>())
{
#ifdef USE_INOTIFY
	pImpl->inotify.SetNonBlock(true);
#endif
}

FileWatcher::~FileWatcher()
{
}

void FileWatcher::add_watch(const fs::path& file_path)
{
	if(has_watch(file_path))
	{
		return;
	}

#ifdef USE_INOTIFY
	pImpl->inotify_watches.emplace_back(std::make_unique<InotifyWatch>(file_path, IN_CLOSE_WRITE));
	pImpl->inotify.Add(pImpl->inotify_watches.back().get());
#endif
}

bool FileWatcher::has_watch(const fs::path& file_path) const
{
#ifdef USE_INOTIFY
	return pImpl->inotify.HasWatch(file_path);
#else
	return false;
#endif
}

std::vector<fs::path> FileWatcher::get_updates()
{
	std::vector<fs::path> paths;

#ifdef USE_INOTIFY
	pImpl->inotify.WaitForEvents();
	if(pImpl->inotify.GetEventCount() == 0)
	{
		return paths;
	}

	InotifyEvent event;
	while(pImpl->inotify.PopEvent(event))
	{
		paths.emplace_back(event.GetWatch()->GetPath());
	}
#endif

	return paths;
}

}
