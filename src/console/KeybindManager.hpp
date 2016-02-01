#pragma once

#include <string>
#include <unordered_map>

class Console;

class KeybindManager
{
	public:
		KeybindManager(const Console* console);

		void bind_key(int key, const std::string& command);
		void unbind_key(int key);

		void keypress(int key) const;

	private:
		const Console* console;
		std::unordered_map<int, std::string> keybinds;
};