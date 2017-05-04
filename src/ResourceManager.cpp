#include "ResourceManager.hpp"

#include <iostream>
#include <sstream>
#include <unordered_map>

#include <easylogging++/easylogging++.hpp>

#include "Game.hpp"
#include "Util.hpp"
#include "console/ArgumentParser.hpp"
#include "graphics/OpenGL/ShaderObject.hpp"
#include "util/filesystem.hpp"
#include "util/FileWatcher.hpp"

using std::cerr;
using std::string;
using Graphics::OpenGL::ShaderObject;

struct ResourceManager::impl
{
	Util::FileWatcher file_watcher;
	std::unordered_map<std::string, std::unique_ptr<Graphics::OpenGL::ShaderObject>> cache_ShaderObject;
};

ResourceManager::ResourceManager()
:
	pImpl(std::make_unique<impl>())
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::check_updates()
{
	std::vector<fs::path> paths = pImpl->file_watcher.get_updates();
	for(const fs::path& path : paths)
	{
		if(path.extension() == ".fs"
		|| path.extension() == ".vs")
		{
			get_ShaderObject(path, true);
		}
	}
}

static std::unordered_map<string, string> parse_block(const string& s)
{
	std::unordered_map<string, string> things;

	std::istringstream ss(s);
	for(std::string line; std::getline(ss, line);)
	{
		std::vector<string> parts = ArgumentParser().parse_args(line);
		if(parts.size() == 0) continue; // comment or empty
		if(parts.size() != 2)
		{
			// TODO
			cerr << "invalid line: " << line << "\n";
			continue;
		}
		things.emplace(parts[0], parts[1]);
	}
	return things;
}

static bool is_block_valid(const std::unordered_map<string, string>& block)
{
	if(block.count("name") != 1) return false;
	if(block.count("shader") != 1) return false;
	return true;
}

void ResourceManager::load_blocks(Game& game)
{
	for(const fs::directory_entry& entry : fs::recursive_directory_iterator("blocks"))
	{
		const fs::path path = entry.path();
		if(path.extension() != ".btblock")
		{
			continue;
		}

		auto block = parse_block(Util::read_file(path));
		if(!is_block_valid(block))
		{
			// TODO
			LOG(WARNING) << "ignoring invalid block " << path.u8string();
			continue;
		}
		game.add_block_2(block["name"], block["shader"]);
	}
}

Resource<ShaderObject> ResourceManager::get_ShaderObject(fs::path path, const bool reload)
{
	GLenum type;
	if(path.extension() == ".fs")
	{
		type = GL_FRAGMENT_SHADER;
	}
	else if(path.extension() == ".vs")
	{
		type = GL_VERTEX_SHADER;
	}
	else
	{
		throw std::invalid_argument("bad shader path: " + path.u8string());
	}
	if(!Util::file_is_openable(path))
	{
		const fs::path orig = path;
		path.replace_filename("default" + path.extension().string()); // no replace_stem?
		if(!Util::file_is_openable(path))
		{
			throw std::runtime_error("shader not found and no default exists: " + orig.u8string());
		}
	}
	auto i = pImpl->cache_ShaderObject.find(path);
	if(i == pImpl->cache_ShaderObject.cend())
	{
		// the shader object is not in the cache, so compile and add it
		i = pImpl->cache_ShaderObject.emplace(path, std::make_unique<ShaderObject>(path, type)).first;
		pImpl->file_watcher.add_watch(path);
	}
	else if(reload)
	{
		std::unique_ptr<ShaderObject> p;
		try
		{
			p = std::make_unique<ShaderObject>(path, type);
		}
		catch(const std::runtime_error& e)
		{
			cerr << "failed to update ShaderObject " << path.u8string() << ": " << e.what() << "\n";
			return Resource<ShaderObject>(&i->second, path);
		}
		std::swap(p, i->second);
		Resource<ShaderObject> r(&i->second, path);
		r.update();
		return r;
	}
	return Resource<ShaderObject>(&i->second, path);
}
