#pragma once

#include <string>

#include "Console.hpp"

class Command
{
	public:
		Command(Console* console, const std::string& name, const console_handler_t& handler);
		~Command();

		Command(const Command&) = delete;
		void operator=(const Command&) = delete;
		Command(Command&&) = default;

	private:
		Console* console;
		const std::string name;
};