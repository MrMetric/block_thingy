#include "Command.hpp"

Command::Command(Console* console, const std::string& name, const console_handler_t& handler)
	:
	console(console),
	name(name)
{
	this->console->add_command(this->name, handler);
}

Command::~Command()
{
	this->console->unadd_command(this->name);
}