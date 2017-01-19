#include "ResourceManager.hpp"

#include <iostream>
#include <unordered_map>

#include "Game.hpp"
#include "Util.hpp"
#include "console/ArgumentParser.hpp"

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

using std::cerr;
using std::string;

namespace ResourceManager {

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

void load_blocks(Game& game)
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

}
