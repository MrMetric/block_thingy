#pragma once

#include <experimental/propagate_const>
#include <memory>
#include <vector>

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
	std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
