#include "ArgumentParser.hpp"

#include <string>
#include <vector>

ArgumentParser::ArgumentParser()
{
}

std::vector<std::string> ArgumentParser::parse_args(const std::string& argline)
{
	std::vector<std::string> args;

	this->iterator = argline.begin();
	this->end = argline.end();

	while(this->iterator != this->end)
	{
		char c = *this->iterator;
		++this->iterator;
		if(c == ' ' || c == '\t')
		{
			continue;
		}
		if(c == '\'' || c == '"')
		{
			args.push_back(this->read_string(c));
			continue;
		}
		args.push_back(c + this->read_string(' '));
	}

	return args;
}

std::string ArgumentParser::read_string(const char endchar)
{
	std::string s = "";
	while(this->iterator != this->end)
	{
		char c = *this->iterator;
		if(c == '\\')
		{
			++this->iterator;
			if(this->iterator == this->end)
			{
				throw truncated_argument("unexpected end of argument string in escape sequence");
			}
			c = *this->iterator;
			++this->iterator;
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
			++this->iterator;
			return s;
		}
		s += c;
		++this->iterator;
	}
	if(endchar != ' ')
	{
		throw truncated_argument("unexpected end of argument string");
	}
	return s;
}