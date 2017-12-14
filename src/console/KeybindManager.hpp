#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

#include "fwd/console/Console.hpp"
#include "fwd/util/key_press.hpp"
#include "fwd/util/mouse_press.hpp"

namespace block_thingy {

class KeybindManager
{
public:
	explicit KeybindManager(Console& console);
	~KeybindManager();

	KeybindManager(KeybindManager&&) = delete;
	KeybindManager(const KeybindManager&) = delete;
	KeybindManager& operator=(KeybindManager&&) = delete;
	KeybindManager& operator=(const KeybindManager&) = delete;

	void bind_key(int key, const std::string& command);
	void bind_key(const std::string& key, const std::string& command);
	void unbind_key(int key);

	void keypress(const util::key_press&);
	void mousepress(const util::mouse_press&);
	void joypress(int joystick, int button, bool pressed);

	static int translate_key(std::string key);

private:
	Console& console;
	std::unordered_map<int, std::string> keybinds;
	std::unordered_map<int, std::string> release_auto;
	std::unordered_map<int, uint_fast32_t> joystate;
};

}
