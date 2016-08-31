#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

// intentionally not a single bit
constexpr int GLFW_CUSTOM_MOD_JOYSTICK = 0x4321234;

#include "fwd/console/Console.hpp"

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

		void keypress(int key, int scancode, int action, int mods);
		void mousepress(int button, int action, int mods);
		void joypress(int joystick, int button, bool pressed);

		// intentionally not const ref
		static int translate_key(std::string key);

	private:
		Console& console;
		std::unordered_map<int, std::string> keybinds;
		std::unordered_map<int, std::string> release_auto;
		std::unordered_map<int, uint_fast32_t> joystate;
};
