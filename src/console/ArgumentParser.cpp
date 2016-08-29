#include "ArgumentParser.hpp"

#include <string>
#include <vector>

using std::string;

ArgumentParser::ArgumentParser()
{
}

std::vector<string> ArgumentParser::parse_args(const string& argline)
{
	std::vector<string> args;

	iterator = argline.cbegin();
	end = argline.cend();

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

string ArgumentParser::read_string(const char endchar)
{
	string s = "";
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
truncated_argument::truncated_argument(const string& what_arg) : std::runtime_error(what_arg){}
truncated_argument::~truncated_argument(){}
