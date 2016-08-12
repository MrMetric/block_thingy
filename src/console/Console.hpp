#pragma once

#include <functional>
#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

class Game;

using console_handler_t = std::function<void(Game&, const std::vector<std::string>& args)>;
using console_handler_noargs_t = std::function<void(Game&)>;

class console_handler_wrapper
{
	public:
		console_handler_wrapper(const console_handler_t&);
		console_handler_wrapper(const console_handler_noargs_t&);

		void operator()(Game&, const std::vector<std::string>& args) const;

	private:
		bool has_args;

		// uses more memory (I assume), but I do not know how to make a union work
		console_handler_t handler_args;
		console_handler_noargs_t handler_noargs;
};

class Console
{
	public:
		Console(Game&);

		Console(Console&&) = delete;
		Console(const Console&) = delete;
		void operator=(const Console&) = delete;

		void add_command(const std::string& name, const console_handler_wrapper& handler);
		void unadd_command(const std::string& name);
		void run_line(const std::string& line);
		void run_command(const std::string& name, const std::string& argline = "") const;
		void run_command(const std::string& name, const std::vector<std::string>& args) const;

		std::ostream& logger;
		std::ostream& error_logger;

	private:
		std::vector<std::string> parse_args(const std::string&);

		Game& game;
		std::unordered_map<std::string, console_handler_wrapper> handlers;
};
