#include "Settings.hpp"

#include <map>
#include <stdexcept>

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
	Game::instance->event_manager.do_event(Event_change_setting(name, &bools[name]));
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
	Game::instance->event_manager.do_event(Event_change_setting(name, &strings[name]));
}


void Settings::load()
{
	// TODO
}

void Settings::save()
{
	// TODO
}
