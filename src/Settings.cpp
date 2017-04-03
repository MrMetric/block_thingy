#include "Settings.hpp"

#include <algorithm>
#include <map>
#include <fstream>
#include <stdexcept>

#include <easylogging++/easylogging++.hpp>

#include "console/Console.hpp"
#include "Game.hpp"
#include "event/type/Event_change_setting.hpp"

using std::string;


static std::map<const string, bool> bools =
{
	{"cull_face", true},
	{"fullscreen", false},
	{"show_debug_info", true},
	{"show_HUD", true},
	{"wireframe", false},
};

template<>
bool Settings::has<bool>(const string& name)
{
	return bools.count(name) > 0;
}

template<>
bool Settings::get(const string& name)
{
	const auto i = bools.find(name);
	if(i == bools.cend())
	{
		// TODO: ?
		throw std::runtime_error("unknown setting name: " + name);
	}
	return i->second;
}

template<>
void Settings::set(const string& name, const bool value)
{
	if(has<bool>(name) && get<bool>(name) == value)
	{
		return;
	}
	bools[name] = value;
	if(Game::instance != nullptr)
	{
		Game::instance->event_manager.do_event(Event_change_setting(name, &bools[name]));
	}
}


static std::map<const string, string> strings =
{
	{"screen_shader", "default"},
};

template<>
bool Settings::has<string>(const string& name)
{
	return strings.count(name) > 0;
}

template<>
string Settings::get(const string& name)
{
	const auto i = strings.find(name);
	if(i == strings.cend())
	{
		// TODO: ?
		throw std::runtime_error("unknown setting name: " + name);
	}
	return i->second;
}

template<>
void Settings::set(const string& name, const string value)
{
	if(has<string>(name) && get<string>(name) == value)
	{
		return;
	}
	strings[name] = std::move(value);
	if(Game::instance != nullptr)
	{
		Game::instance->event_manager.do_event(Event_change_setting(name, &strings[name]));
	}
}


void Settings::add_command_handlers()
{
	Console::instance->add_command("set_bool", {[](const std::vector<string>& args)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_bool <name> <value>";
			return;
		}

		const string value_str = args[1];
		if(value_str != "true" && value_str != "false")
		{
			LOG(ERROR) << "Invalid bool value (must be \"true\" or \"false\")";
			return;
		}
		const string name = args[0];
		const bool value = (value_str == "true");
		Settings::set(name, value);
		if(Game::instance != nullptr) // not called from initial Settings::load()
		{
			LOG(INFO) << "set bool: " << name << " = " << (value ? "true" : "false");
		}
	}});
	Console::instance->add_command("toggle_bool", {[](const std::vector<string>& args)
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
		if(Game::instance != nullptr)
		{
			LOG(INFO) << "set bool: " << name << " = " << (value ? "true" : "false");
		}
	}});
	Console::instance->add_command("set_string", {[](const std::vector<string>& args)
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: set_string <name> <value>";
			return;
		}

		const string name = args[0];
		const string value = args[1];
		Settings::set<string>(name, value);
		if(Game::instance != nullptr)
		{
			LOG(INFO) << "set string: " << name << " = " << value;
		}
	}});
}

void Settings::load()
{
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
	for(const auto& p : bools)
	{
		const string name = p.first;
		const bool value = p.second;

		f << "set_bool " << format_string(name) << ' ' << (value ? "true" : "false") << '\n';
	}
	for(const auto& p : strings)
	{
		const string name = p.first;
		const string value = p.second;

		f << "set_string " << format_string(name) << ' ' << format_string(value) << '\n';
	}
}
