#include "Console.hpp"

#include <stdexcept>
#include <vector>

#include "ArgumentParser.hpp"

Console::Console()
{
}

console_handler_wrapper::console_handler_wrapper(console_handler_t handler)
	:
	args(true),
	handler_args(handler)
{
}

console_handler_wrapper::console_handler_wrapper(console_handler_noargs_t handler)
	:
	args(false),
	handler_noargs(handler)
{
}

void console_handler_wrapper::operator()(const std::vector<std::string>& args) const
{
	if(this->args)
	{
		this->handler_args(args);
	}
	else
	{
		this->handler_noargs();
	}
}

void Console::add_command(const std::string& name, const console_handler_wrapper& handler)
{
	if(!this->handlers.insert({name, handler}).second)
	{
		throw std::runtime_error("tried to add duplicate command: "  + name);
	}
}

void Console::unadd_command(const std::string& name)
{
	this->handlers.erase(name);
}

void Console::run_line(const std::string& line)
{
	if(line.length() == 0)
	{
		return;
	}
	std::vector<std::string> args = ArgumentParser().parse_args(line);
	if(args.size() < 1)
	{
		return;
	}
	std::string name = args[0];
	args.erase(args.begin());
	this->run_command(name, args);
}

void Console::run_command(const std::string& name, const std::string& argline) const
{
	std::vector<std::string> args = ArgumentParser().parse_args(argline);
	this->run_command(name, args);
}

void Console::run_command(const std::string& name, const std::vector<std::string>& args) const
{
	if(name[0] == '#') // ignore comments
	{
		return;
	}
	auto i = this->handlers.find(name);
	if(i == this->handlers.end())
	{
		throw std::runtime_error("unknown command: " + name);
	}
	i->second(args);
}
