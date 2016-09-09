#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

#include "fwd/console/Console.hpp"
#include "fwd/util/key_mods.hpp"

class KeybindManager
{
	public:
		explicit KeybindManager(Console& console);
		~KeybindManager();

		KeybindManager(KeybindManager&&) = delete;
		KeybindManager(const KeybindManager&) = delete;
		void operator=(const KeybindManager&) = delete;

		void bind_key(int key, const std::string& command);
		void bind_key(const std::string& key, const std::string& command);
		void unbind_key(int key);

		void keypress(int key, int scancode, int action, Util::key_mods);
		void mousepress(int button, int action, Util::key_mods);
		void joypress(int joystick, int button, bool pressed);

		// intentionally not const ref
		static int translate_key(std::string key);

	private:
		Console& console;
		std::unordered_map<int, std::string> keybinds;
		std::unordered_map<int, std::string> release_auto;
		std::unordered_map<int, uint_fast32_t> joystate;
};
