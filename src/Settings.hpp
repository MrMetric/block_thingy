#pragma once

#include <stdint.h>
#include <string>

#include <strict_variant/variant.hpp>

struct Settings
{
	using value_t = strict_variant::variant<bool, double, int64_t, std::string>;

	template<typename T>
	static bool has(const std::string& name);

	template<typename T>
	static T get(const std::string& name);

	template<typename T>
	static void set(const std::string& name, T);

	static bool has(const std::string& name);
	static value_t get(const std::string& name);
	static void set(const std::string& name, value_t);

	static void load();
	static void save();

private:
	friend int main(int, char**);
	static void add_command_handlers();
};
