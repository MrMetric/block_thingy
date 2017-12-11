#include "Command.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

using std::string;

Command::Command
(
	Console& console,
	const string& name,
	const console_handler_t& handler
)
:
	console(console),
	name(name)
{
	if(name.empty())
	{
		throw std::invalid_argument("command name can not be empty");
	}
	console.add_command(name, handler);
}

Command::~Command()
{
	if(!name.empty())
	{
		console.unadd_command(name);
	}
}

Command::Command(Command&& that)
:
	console(that.console),
	name(std::move(that.name))
{
	assert(!name.empty());
	assert(that.name.empty());
}
