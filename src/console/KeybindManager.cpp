#include "KeybindManager.hpp"

#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>

#include "Console.hpp"
#include "Game.hpp"
#include "Util.hpp"
#include "util/key_mods.hpp"

using std::string;

KeybindManager::KeybindManager(Console& console)
	:
	console(console)
{
	console.add_command("bind", {[&keybinder=*this](Game& game, const std::vector<string>& args)
	{
		if(args.size() != 2)
		{
			game.console.error_logger << "Usage: bind <key> <command>\n";
			return;
		}
		keybinder.bind_key(args[0], args[1]);
	}});
	console.add_command("unbind", {[&keybinder=*this](Game& game, const std::vector<string>& args)
	{
		if(args.size() != 1)
		{
			game.console.error_logger << "Usage: unbind <key>\n";
			return;
		}
		int key = KeybindManager::translate_key(args[0]);
		keybinder.unbind_key(key);
	}});
}

KeybindManager::~KeybindManager()
{
	console.unadd_command("bind");
	console.unadd_command("unbind");
}

void KeybindManager::bind_key(const int key, const string& command)
{
	keybinds[key] = command;
}

void KeybindManager::bind_key(const string& key_string, const string& command)
{
	int key = KeybindManager::translate_key(key_string);
	if(key == GLFW_KEY_UNKNOWN)
	{
		console.error_logger << "unknown key name: " << key_string << "\n";
	}
	bind_key(key, command);
}

void KeybindManager::unbind_key(const int key)
{
	keybinds.erase(key);
}

void KeybindManager::keypress(const int key, const int scancode, const int action, const Util::key_mods mods)
{
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		const auto i = keybinds.find(key);
		if(i != keybinds.cend())
		{
			const string command = i->second;
			if(command[0] == '+')
			{
				if(action == GLFW_REPEAT)
				{
					return;
				}
				string command2 = command;
				command2[0] = '-';
				release_auto[key] = command2;
			}
			console.run_line(command);
		}
	}
	else if(action == GLFW_RELEASE)
	{
		const auto i = release_auto.find(key);
		if(i != release_auto.cend())
		{
			console.run_line(i->second);
		}
	}
	else
	{
		console.error_logger << "unknown keypress action: " << action << "\n";
	}
}

void KeybindManager::mousepress(const int button, const int action, const Util::key_mods mods)
{
	keypress(button, 0, action, mods);
}

void KeybindManager::joypress(const int joystick, const int button, const bool pressed)
{
	const auto i = joystate.find(button);
	if(i == joystate.cend())
	{
		joystate[button] = 0;
	}

	const int key = 1000 * joystick + button;

	if(!pressed)
	{
		joystate[button] = 0;
		keypress(key, 0, GLFW_RELEASE, Util::key_mods(0));
		return;
	}

	++joystate[button];
	if(joystate[button] == 1)
	{
		keypress(key, 0, GLFW_PRESS, Util::key_mods(0));
		return;
	}

	if(joystate[button] >= 30 && joystate[button] % 15 == 0)
	{
		keypress(key, 0, GLFW_REPEAT, Util::key_mods(0));
		return;
	}
}

// I think this is too long :[
int KeybindManager::translate_key(string key)
{
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	if(key.length() == 1)
	{
		switch(key[0])
		{
			case ' ': return GLFW_KEY_SPACE;
			case '\'': return GLFW_KEY_APOSTROPHE;
			case ',': return GLFW_KEY_COMMA;
			case '-': return GLFW_KEY_MINUS;
			case '.': return GLFW_KEY_PERIOD;
			case '/': return GLFW_KEY_SLASH;
			case '0': return GLFW_KEY_0;
			case '1': return GLFW_KEY_1;
			case '2': return GLFW_KEY_2;
			case '3': return GLFW_KEY_3;
			case '4': return GLFW_KEY_4;
			case '5': return GLFW_KEY_5;
			case '6': return GLFW_KEY_6;
			case '7': return GLFW_KEY_7;
			case '8': return GLFW_KEY_8;
			case '9': return GLFW_KEY_9;
			case ';': return GLFW_KEY_SEMICOLON;
			case '=': return GLFW_KEY_EQUAL;
			case 'a': return GLFW_KEY_A;
			case 'b': return GLFW_KEY_B;
			case 'c': return GLFW_KEY_C;
			case 'd': return GLFW_KEY_D;
			case 'e': return GLFW_KEY_E;
			case 'f': return GLFW_KEY_F;
			case 'g': return GLFW_KEY_G;
			case 'h': return GLFW_KEY_H;
			case 'i': return GLFW_KEY_I;
			case 'j': return GLFW_KEY_J;
			case 'k': return GLFW_KEY_K;
			case 'l': return GLFW_KEY_L;
			case 'm': return GLFW_KEY_M;
			case 'n': return GLFW_KEY_N;
			case 'o': return GLFW_KEY_O;
			case 'p': return GLFW_KEY_P;
			case 'q': return GLFW_KEY_Q;
			case 'r': return GLFW_KEY_R;
			case 's': return GLFW_KEY_S;
			case 't': return GLFW_KEY_T;
			case 'u': return GLFW_KEY_U;
			case 'v': return GLFW_KEY_V;
			case 'w': return GLFW_KEY_W;
			case 'x': return GLFW_KEY_X;
			case 'y': return GLFW_KEY_Y;
			case 'z': return GLFW_KEY_Z;
			case '[': return GLFW_KEY_LEFT_BRACKET;
			case '\\': return GLFW_KEY_BACKSLASH;
			case ']': return GLFW_KEY_RIGHT_BRACKET;
			case '`': return GLFW_KEY_GRAVE_ACCENT;
			case '\t': return GLFW_KEY_TAB;
			default: return GLFW_KEY_UNKNOWN;
		}
	}

	if(Util::string_starts_with(key, "mouse"))
	{
		if(key.length() != 6)
		{
			return GLFW_KEY_UNKNOWN;
		}
		int8_t btn = key[5] - '1';
		if(btn < 0 || btn > 7)
		{
			return GLFW_KEY_UNKNOWN;
		}
		return GLFW_MOUSE_BUTTON_1 + btn;
	}

	if(key == "space") return GLFW_KEY_SPACE;
	if(key == "tab") return GLFW_KEY_TAB;
	if(key == "esc" || key == "escape") return GLFW_KEY_ESCAPE;
	if(key == "enter" || key == "return") return GLFW_KEY_ENTER;
	if(key == "backspace") return GLFW_KEY_BACKSPACE;
	if(key == "insert") return GLFW_KEY_INSERT;
	if(key == "delete") return GLFW_KEY_DELETE;
	if(key == "right") return GLFW_KEY_RIGHT;
	if(key == "left") return GLFW_KEY_LEFT;
	if(key == "down") return GLFW_KEY_DOWN;
	if(key == "up") return GLFW_KEY_UP;
	if(key == "page_up" || key == "pgdn") return GLFW_KEY_PAGE_UP;
	if(key == "page_down" || key == "pgup") return GLFW_KEY_PAGE_DOWN;
	if(key == "home") return GLFW_KEY_HOME;
	if(key == "end") return GLFW_KEY_END;
	if(key == "caps_lock") return GLFW_KEY_CAPS_LOCK;
	if(key == "scroll_lock") return GLFW_KEY_SCROLL_LOCK;
	if(key == "num_lock") return GLFW_KEY_NUM_LOCK;
	if(key == "print_screen") return GLFW_KEY_PRINT_SCREEN;
	if(key == "pause") return GLFW_KEY_PAUSE;
	if(key == "f1") return GLFW_KEY_F1;
	if(key == "f2") return GLFW_KEY_F2;
	if(key == "f3") return GLFW_KEY_F3;
	if(key == "f4") return GLFW_KEY_F4;
	if(key == "f5") return GLFW_KEY_F5;
	if(key == "f6") return GLFW_KEY_F6;
	if(key == "f7") return GLFW_KEY_F7;
	if(key == "f8") return GLFW_KEY_F8;
	if(key == "f9") return GLFW_KEY_F9;
	if(key == "f10") return GLFW_KEY_F10;
	if(key == "f11") return GLFW_KEY_F11;
	if(key == "f12") return GLFW_KEY_F12;
	if(key == "f13") return GLFW_KEY_F13;
	if(key == "f14") return GLFW_KEY_F14;
	if(key == "f15") return GLFW_KEY_F15;
	if(key == "f16") return GLFW_KEY_F16;
	if(key == "f17") return GLFW_KEY_F17;
	if(key == "f18") return GLFW_KEY_F18;
	if(key == "f19") return GLFW_KEY_F19;
	if(key == "f20") return GLFW_KEY_F20;
	if(key == "f21") return GLFW_KEY_F21;
	if(key == "f22") return GLFW_KEY_F22;
	if(key == "f23") return GLFW_KEY_F23;
	if(key == "f24") return GLFW_KEY_F24;
	if(key == "f25") return GLFW_KEY_F25;
	if(key == "kp0" || key == "kp_0") return GLFW_KEY_KP_0;
	if(key == "kp1" || key == "kp_1") return GLFW_KEY_KP_1;
	if(key == "kp2" || key == "kp_2") return GLFW_KEY_KP_2;
	if(key == "kp3" || key == "kp_3") return GLFW_KEY_KP_3;
	if(key == "kp4" || key == "kp_4") return GLFW_KEY_KP_4;
	if(key == "kp5" || key == "kp_5") return GLFW_KEY_KP_5;
	if(key == "kp6" || key == "kp_6") return GLFW_KEY_KP_6;
	if(key == "kp7" || key == "kp_7") return GLFW_KEY_KP_7;
	if(key == "kp8" || key == "kp_8") return GLFW_KEY_KP_8;
	if(key == "kp9" || key == "kp_9") return GLFW_KEY_KP_9;
	if(key == "kp_decimal") return GLFW_KEY_KP_DECIMAL;
	if(key == "kp_divide") return GLFW_KEY_KP_DIVIDE;
	if(key == "kp_multiply") return GLFW_KEY_KP_MULTIPLY;
	if(key == "kp_subtract") return GLFW_KEY_KP_SUBTRACT;
	if(key == "kp_add") return GLFW_KEY_KP_ADD;
	if(key == "kp_enter") return GLFW_KEY_KP_ENTER;
	if(key == "kp_equal") return GLFW_KEY_KP_EQUAL;
	if(key == "lshift") return GLFW_KEY_LEFT_SHIFT;
	if(key == "lctrl" || key == "lcontrol") return GLFW_KEY_LEFT_CONTROL;
	if(key == "lalt") return GLFW_KEY_LEFT_ALT;
	if(key == "lsuper") return GLFW_KEY_LEFT_SUPER;
	if(key == "rshift") return GLFW_KEY_RIGHT_SHIFT;
	if(key == "rctrl" || key == "rcontrol") return GLFW_KEY_RIGHT_CONTROL;
	if(key == "ralt") return GLFW_KEY_RIGHT_ALT;
	if(key == "rsuper") return GLFW_KEY_RIGHT_SUPER;
	if(key == "menu") return GLFW_KEY_MENU;

	if(key == "joy_xbox360_a") return 1000;
	if(key == "joy_xbox360_b") return 1001;
	if(key == "joy_xbox360_x") return 1002;
	if(key == "joy_xbox360_y") return 1003;
	if(key == "joy_xbox360_lb" || key == "joy_xbox360_l1") return 1004;
	if(key == "joy_xbox360_rb" || key == "joy_xbox360_r1") return 1005;
	if(key == "joy_xbox360_back") return 1006;
	if(key == "joy_xbox360_start") return 1007;
	if(key == "joy_xbox360_logo") return 1008; // TODO: check name (player changer?)
	if(key == "joy_xbox360_analog_left" || key == "joy_xbox360_l3") return 1009;
	if(key == "joy_xbox360_analog_right" || key == "joy_xbox360_r3") return 1010;

	if(key == "joy_ps3_select") return 1000;
	if(key == "joy_ps3_l3") return 1001;
	if(key == "joy_ps3_r3") return 1002;
	if(key == "joy_ps3_start") return 1003;
	if(key == "joy_ps3_up") return 1004;
	if(key == "joy_ps3_right") return 1005;
	if(key == "joy_ps3_down") return 1006;
	if(key == "joy_ps3_left") return 1007;
	if(key == "joy_ps3_l2") return 1008;
	if(key == "joy_ps3_r2") return 1009;
	if(key == "joy_ps3_l1") return 1010;
	if(key == "joy_ps3_r1") return 1011;
	if(key == "joy_ps3_triangle") return 1012;
	if(key == "joy_ps3_circle") return 1013;
	if(key == "joy_ps3_x") return 1014;
	if(key == "joy_ps3_square") return 1015;
	if(key == "joy_ps3_logo") return 1016; // TODO: check name

	return GLFW_KEY_UNKNOWN;
}
