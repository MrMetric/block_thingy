#pragma once

#include <string>
#include <unordered_map>

class Console;

class KeybindManager
{
	public:
		explicit KeybindManager(Console& console);
		~KeybindManager();

		void bind_key(int key, const std::string& command);
		void bind_key(const std::string& key, const std::string& command);
		void unbind_key(int key);

		void keypress(int key, int action);

		static int translate_key(std::string key);

	private:
		Console& console;
		std::unordered_map<int, std::string> keybinds;
		std::unordered_map<int, std::string> release_auto;
};
