#include "Settings.hpp"

#include <algorithm>
#include <fstream>
#include <limits>
#include <map>
#include <stdexcept>

#include <easylogging++/easylogging++.hpp>

#include "console/Console.hpp"
#include "Game.hpp"
#include "event/type/Event_change_setting.hpp"

using std::string;

template<typename T>
static std::map<string, T>& get_map()
{
	static std::map<string, T> map;
	return map;
}

template<typename T>
bool Settings::has(const string& name)
{
	return get_map<T>().count(name) > 0;
}

template<typename T>
T Settings::get(const string& name)
{
	const auto i = get_map<T>().find(name);
	if(i == get_map<T>().cend())
	{
		// TODO: ?
		throw std::runtime_error("unknown setting name: " + name);
	}
	return i->second;
}

template<typename T>
void Settings::set(const string& name, T value)
{
	if(has<T>(name) && get<T>(name) == value)
	{
		return;
	}
	const T old_value = std::move(get_map<T>()[name]);
	get_map<T>()[name] = std::move(value);
	if(Game::instance != nullptr)
	{
		Game::instance->event_manager.do_event(Event_change_setting(name, old_value, get_map<T>()[name]));
	}
}

void Settings::load()
{
	get_map<bool>() =
	{
		{"cull_face", true},
		{"fullscreen", false},
		{"show_chunk_outlines", false},
		{"show_container_bounds", false},
		{"show_debug_info", false},
		{"show_HUD", true},
		{"wireframe", false},
	};
	get_map<double>() =
	{
		{"cursor_blink_rate", 0.5}, // TODO: ensure > 0
		{"far_plane", 1500},
		{"fov", 75},
		{"min_light", 0.005},
		{"near_plane", 0.1},
		{"ortho_size", 6},
	};
	get_map<int64_t>() =
	{
		{"font_size", 24},
		{"light_smoothing", 2},
	};
	get_map<string>() =
	{
		{"font", "fonts/Anonymous Pro/Anonymous Pro.ttf"},
		{"mesher", "Simple"},
		{"projection_type", "default"},
		{"screen_shader", "default"},
	};

	Console::instance->run_line("exec settings");
}

// TODO: use smallest representation by using quoted strings
static string format_string(string s)
{
	if(s.empty())
	{
		return "''";
	}
	if(s[0] == '\'' || s[0] == '"')
	{
		s = '\\' + s;
	}
	string s2;
	for(const char c : s)
	{
		     if(c == ' ' ) s2 += "\\ ";
		else if(c == '\t') s2 += "\\t";
		else if(c == '\r') s2 += "\\r";
		else if(c == '\n') s2 += "\\n";
		else
		{
			s2 += c;
		}
	}
	return s2;
}

void Settings::save()
{
	std::ofstream f("scripts/settings");
	f.precision(std::numeric_limits<double>::max_digits10);
	for(const auto& p : get_map<bool>())
	{
		f << "set_bool " << format_string(p.first) << ' ' << (p.second ? "true" : "false") << '\n';
	}
	for(const auto& p : get_map<double>())
	{
		f << "set_float " << format_string(p.first) << ' ' << p.second << '\n';
	}
	for(const auto& p : get_map<int64_t>())
	{
		f << "set_int " << format_string(p.first) << ' ' << p.second << '\n';
	}
	for(const auto& p : get_map<string>())
	{
		f << "set_string " << format_string(p.first) << ' ' << format_string(p.second) << '\n';
	}
}

void Settings::add_command_handlers()
{
	Console::instance->add_command("set_bool", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_bool <name> <value>";
			return;
		}

		const string name = args[0];
		const string value_str = args[1];
		if(value_str != "true" && value_str != "false")
		{
			LOG(ERROR) << "Invalid bool value for " << name << " (must be \"true\" or \"false\")";
			return;
		}
		const bool value = (value_str == "true");
		Settings::set(name, value);
		if(Game::instance != nullptr) // not called from initial Settings::load()
		{
			LOG(INFO) << "set bool: " << name << " = " << (value ? "true" : "false");
		}
	}});
	Console::instance->add_command("toggle_bool", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: toggle_bool <setting name>";
			return;
		}

		const string name = args[0];
		if(!Settings::has<bool>(name))
		{
			LOG(ERROR) << "Unknown bool name: " << name;
			return;
		}
		bool value = Settings::get<bool>(name);
		value = !value;
		Settings::set(name, value);
		if(Game::instance != nullptr) // not called from initial Settings::load()
		{
			LOG(INFO) << "set bool: " << name << " = " << (value ? "true" : "false");
		}
	}});
	Console::instance->add_command("set_float", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_float <name> <value or + difference>";
			return;
		}

		const string name = args[0];
		const string svalue = args[1];
		double value = Settings::has<double>(name) ? Settings::get<double>(name) : 0;
		if(svalue[0] == '+')
		{
			value += std::stod(svalue.substr(1));
		}
		else
		{
			value = std::stod(svalue);
		}

		if(name == "fov")
		{
			if(value < 0)
			{
				value = std::fmod(value, 360) + 360;
			}
			else
			{
				value = std::fmod(value, 360);
			}
			if(value == 0) // avoid division by zero
			{
				value = 360;
			}
		}

		Settings::set<double>(name, value);
		if(Game::instance != nullptr) // not called from initial Settings::load()
		{
			LOG(INFO) << "set float: " << name << " = " << value;
		}
	}});
	Console::instance->add_command("set_int", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_int <name> <value or + difference>";
			return;
		}

		const string name = args[0];
		const string svalue = args[1];
		int64_t value = Settings::has<int64_t>(name) ? Settings::get<int64_t>(name) : 0;
		if(svalue[0] == '+')
		{
			value += std::stoll(svalue.substr(1));
		}
		else
		{
			value = std::stoll(svalue);
		}

		Settings::set<int64_t>(name, value);
		if(Game::instance != nullptr) // not called from initial Settings::load()
		{
			LOG(INFO) << "set int: " << name << " = " << value;
		}
	}});
	Console::instance->add_command("set_string", {[]
	(
		const std::vector<string>& args
	)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_string <name> <value>";
			return;
		}

		const string name = args[0];
		const string value = args[1];
		Settings::set<string>(name, value);
		if(Game::instance != nullptr) // not called from initial Settings::load()
		{
			LOG(INFO) << "set string: " << name << " = " << value;
		}
	}});
}
