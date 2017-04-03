#include "Console.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

#include <easylogging++/easylogging++.hpp>

#include "ArgumentParser.hpp"

using std::string;

console_handler_wrapper::console_handler_wrapper(const console_handler_t& handler)
:
	has_args(true),
	handler_args(handler)
{
}

console_handler_wrapper::console_handler_wrapper(const console_handler_noargs_t& handler)
:
	has_args(false),
	handler_noargs(handler)
{
}

void console_handler_wrapper::operator()(const std::vector<string>& args) const
{
	if(has_args)
	{
		handler_args(args);
	}
	else
	{
		handler_noargs();
	}
}

Console* Console::instance = nullptr;

Console::Console()
{
	Console::instance = this;

	add_command("exec", {[this](const std::vector<string>& args)
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: exec <string: filename>";
			return;
		}
		const string name = args[0];
		std::ifstream file("scripts/" + name);
		if(!file.is_open())
		{
			LOG(ERROR) << "script not found: " << name;
			return;
		}
		for(string line; std::getline(file, line); )
		{
			run_line(line);
		}
	}});
}

void Console::add_command(const string& name, const console_handler_wrapper& handler)
{
	if(!handlers.insert({name, handler}).second)
	{
		throw std::runtime_error("tried to add duplicate command: "  + name);
	}
}

void Console::unadd_command(const string& name)
{
	handlers.erase(name);
}

void Console::run_line(const string& line)
{
	if(line.length() == 0)
	{
		return;
	}
	std::vector<string> args = ArgumentParser().parse_args(line);
	if(args.size() < 1)
	{
		return;
	}
	const string name = args[0];
	args.erase(args.cbegin());
	run_command(name, args);
}

void Console::run_command(const string& name, const string& argline) const
{
	if(name[0] == '#') // ignore comments
	{
		return;
	}

	const std::vector<string> args = ArgumentParser().parse_args(argline);
	run_command(name, args);
}

void Console::run_command(const string& name, const std::vector<string>& args) const
{
	if(name[0] == '#') // ignore comments
	{
		return;
	}

	const auto i = handlers.find(name);
	if(i == handlers.cend())
	{
		LOG(ERROR) << "unknown command: " << name;
		return;
	}
	i->second(args);
}
