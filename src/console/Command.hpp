#pragma once

#include <string>

#include "console/Console.hpp"

class Command
{
public:
	Command
	(
		Console& console,
		const std::string& name,
		const console_handler_t& handler
	);
	~Command();

	Command(Command&&);
	Command(const Command&) = delete;
	Command& operator=(Command&&) = delete;
	Command& operator=(const Command&) = delete;

private:
	Console& console;
	std::string name;
};
