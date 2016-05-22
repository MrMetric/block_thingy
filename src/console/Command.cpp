#include "Command.hpp"

#include "Console.hpp"

using std::string;

Command::Command(Console& console, const string& name, const console_handler_t& handler)
	:
	console(console),
	name(name)
{
	console.add_command(name, handler);
}

Command::Command(Console& console, const string& name, const console_handler_noargs_t& handler)
	:
	console(console),
	name(name)
{
	console.add_command(name, handler);
}

Command::~Command()
{
	if(unadd)
	{
		console.unadd_command(name);
	}
}

Command::Command(Command&& that)
	:
	console(that.console),
	name(that.name)
{
	that.unadd = false;
}
