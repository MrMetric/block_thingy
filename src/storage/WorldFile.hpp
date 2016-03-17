#pragma once

#include <string>

class World;

class WorldFile
{
	public:
		WorldFile(const std::string& file_path, World& world);

		void load();
		void save();

	private:
		World& world;
};
