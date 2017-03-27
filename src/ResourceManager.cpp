#include "ResourceManager.hpp"

#include <iostream>
#include <sstream>
#include <unordered_map>

#include <inotify-cxx/inotify-cxx.hpp>

#include "Game.hpp"
#include "Util.hpp"
#include "console/ArgumentParser.hpp"
#include "graphics/OpenGL/ShaderObject.hpp"

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif

using std::cerr;
using std::string;
using Graphics::OpenGL::ShaderObject;

static Inotify inotify;
// unique_ptr to keep validity when vector resizes itself
static std::vector<std::unique_ptr<InotifyWatch>> inotify_watches;

static std::unordered_map<std::string, std::unique_ptr<Graphics::OpenGL::ShaderObject>> cache_ShaderObject;

ResourceManager::ResourceManager()
{
	inotify.SetNonBlock(true);
}

ResourceManager::~ResourceManager()
{
	cache_ShaderObject.clear();
}

void ResourceManager::check_updates()
{
	inotify.WaitForEvents();
	if(inotify.GetEventCount() == 0)
	{
		return;
	}

	InotifyEvent event;
	while(inotify.PopEvent(event))
	{
		const string path = event.GetWatch()->GetPath();
		get_ShaderObject(path, true);
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
	for(const auto& entry : fs::recursive_directory_iterator("blocks"))
	{
		string path = entry.path().u8string();
		if(!Util::string_ends_with(path, ".btblock")) continue;
		auto block = parse_block(Util::read_file(path));
		if(!is_block_valid(block))
		{
			// TODO
			cerr << "ignoring invalid block " << path << "\n";
			continue;
		}
		game.add_block_2(block["name"], block["shader"]);
	}
}

Resource<ShaderObject> ResourceManager::get_ShaderObject(string path, const bool reload)
{
	GLenum type;
	Util::path path_parts = Util::split_path(path);
	if(path_parts.ext == "vs")
	{
		type = GL_VERTEX_SHADER;
	}
	else if(path_parts.ext ==  "fs")
	{
		type = GL_FRAGMENT_SHADER;
	}
	else
	{
		throw std::invalid_argument("bad shader path: " + path);
	}
	if(!Util::file_is_openable(path))
	{
		path_parts.file = "default";
		const string path2 = Util::join_path(path_parts);
		if(!Util::file_is_openable(path2))
		{
			throw std::runtime_error("shader not found and no default exists: " + path);
		}
		path = path2;
	}
	auto i = cache_ShaderObject.find(path);
	if(i == cache_ShaderObject.cend())
	{
		// the shader object is not in the cache, so compile and add it
		i = cache_ShaderObject.emplace(path, std::make_unique<ShaderObject>(path, type)).first;
		inotify_watches.emplace_back(std::make_unique<InotifyWatch>(path, IN_CLOSE_WRITE));
		inotify.Add(inotify_watches.back().get());
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
			cerr << "failed to update ShaderObject " << path << ": " << e.what() << "\n";
			return Resource<ShaderObject>(&i->second, path);
		}
		std::swap(p, i->second);
		Resource<ShaderObject> r(&i->second, path);
		r.update();
		return r;
	}
	return Resource<ShaderObject>(&i->second, path);
}
