#pragma once

#include <string>

struct Settings
{
	template<typename T>
	static bool has(const std::string& name);

	template<typename T>
	static T get(const std::string& name);

	template<typename T>
	static void set(const std::string& name, T);

private:
	friend int main(int, char**);
	static void load();
	static void save();
};

template<> bool Settings::has<bool>(const std::string& name);
template<> bool Settings::get(const std::string& name);
template<> void Settings::set(const std::string& name, bool);

template<> bool Settings::has<std::string>(const std::string& name);
template<> std::string Settings::get(const std::string& name);
template<> void Settings::set(const std::string& name, std::string);
