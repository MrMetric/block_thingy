#include "ArgumentParser.hpp"

#include <string>
#include <vector>

ArgumentParser::ArgumentParser()
{
}

std::vector<std::string> ArgumentParser::parse_args(const std::string& argline)
{
	std::vector<std::string> args;

	iterator = argline.begin();
	end = argline.end();

	while(iterator != end)
	{
		char c = *iterator;
		++iterator;
		if(c == ' ' || c == '\t')
		{
			continue;
		}
		if(c == '\'' || c == '"')
		{
			args.push_back(read_string(c));
			continue;
		}
		args.push_back(c + read_string(' '));
	}

	return args;
}

std::string ArgumentParser::read_string(const char endchar)
{
	std::string s = "";
	while(iterator != end)
	{
		char c = *iterator;
		if(c == '\\')
		{
			++iterator;
			if(iterator == end)
			{
				throw truncated_argument("unexpected end of argument string in escape sequence");
			}
			c = *iterator;
			++iterator;
			if(c == 'n')
			{
				s += '\n';
			}
			else if(c == 't')
			{
				s += '\t';
			}
			else
			{
				// TODO: throw exception on bad escape sequence?
				s += c;
			}
			continue;
		}
		if(c == endchar)
		{
			++iterator;
			return s;
		}
		s += c;
		++iterator;
	}
	if(endchar != ' ')
	{
		throw truncated_argument("unexpected end of argument string");
	}
	return s;
}

truncated_argument::truncated_argument(const char* what_arg) : std::runtime_error(what_arg){}
truncated_argument::truncated_argument(const std::string& what_arg) : std::runtime_error(what_arg){}
truncated_argument::~truncated_argument(){}
