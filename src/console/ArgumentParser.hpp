#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace block_thingy {

class ArgumentParser
{
public:
	ArgumentParser() = default;

	ArgumentParser(ArgumentParser&&) = delete;
	ArgumentParser(const ArgumentParser&) = delete;
	ArgumentParser& operator=(ArgumentParser&&) = delete;
	ArgumentParser& operator=(const ArgumentParser&) = delete;

	std::vector<std::string> parse_args(const std::string& argline);

private:
	std::string::const_iterator iterator;
	std::string::const_iterator end;

	std::string read_string(char endchar);
};

class truncated_argument : public std::runtime_error
{
public:
	explicit truncated_argument(const char* what_arg);
	explicit truncated_argument(const std::string& what_arg);
	truncated_argument(const truncated_argument&) = default;
	virtual ~truncated_argument();
};

}
