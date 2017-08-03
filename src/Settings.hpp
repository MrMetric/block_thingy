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

	static void load();
	static void save();

private:
	friend int main(int, char**);
	static void add_command_handlers();
};
