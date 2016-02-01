#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

using console_handler_t = std::function<void(const std::vector<std::string>& args)>;

class Console
{
	public:
		Console();

		void add_command(const std::string& name, const console_handler_t& handler);
		void unadd_command(const std::string& name);
		void run_command(const std::string& name, const std::string& argline = "");
		void run_command(const std::string& name, const std::vector<std::string>& args);

	private:
		std::vector<std::string> parse_args(const std::string& argline);

		std::unordered_map<std::string, console_handler_t> handlers;
};