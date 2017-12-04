#pragma once

#include <memory>
#include <vector>

#include "shim/propagate_const.hpp"
#include "util/filesystem.hpp"

namespace Util {

class FileWatcher
{
public:
	FileWatcher();
	~FileWatcher();

	void add_watch(const fs::path&);
	bool has_watch(const fs::path&) const;
	std::vector<fs::path> get_updates();

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
