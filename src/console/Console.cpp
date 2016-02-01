#include "Console.hpp"

#include <vector>

#include "ArgumentParser.hpp"

Console::Console()
{
}

void Console::add_command(const std::string& name, const console_handler_t& handler)
{
	if(!this->handlers.insert({name, handler}).second)
	{
		// throw something("duplicate command");
	}
}

void Console::unadd_command(const std::string& name)
{
	this->handlers.erase(name);
}

void Console::run_command(const std::string& name, const std::string& argline)
{
	std::vector<std::string> args = ArgumentParser().parse_args(argline);
	this->run_command(name, args);
}

void Console::run_command(const std::string& name, const std::vector<std::string>& args)
{
	this->handlers[name](args);
}