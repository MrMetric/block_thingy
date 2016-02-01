#pragma once

#include <string>
#include <unordered_map>

class Console;

class KeybindManager
{
	public:
		explicit KeybindManager(const Console* console);

		void bind_key(int key, const std::string& command);
		void unbind_key(int key);

		void keypress(int key, int action);

	private:
		const Console* console;
		std::unordered_map<int, std::string> keybinds;
		std::unordered_map<int, std::string> release_auto;
};