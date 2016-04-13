#pragma once

#include <string>

#include "Console.hpp"

class Command
{
	public:
		Command(Console& console, const std::string& name, const console_handler_t& handler);
		Command(Console& console, const std::string& name, const console_handler_noargs_t& handler);
		Command(Command&& that);
		~Command();

		Command(const Command&) = delete;
		void operator=(const Command&) = delete;

	private:
		Console& console;
		const std::string name;
		bool unadd = true;
};
