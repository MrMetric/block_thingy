#include "ArgumentParser.hpp"

using std::string;

namespace block_thingy {

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
		const char c = *iterator;
		if(c == ' ' || c == '\t')
		{
			++iterator;
			continue;
		}
		if(c == '\'' || c == '"')
		{
			++iterator;
			args.push_back(read_string(c));
			continue;
		}
		args.push_back(read_string(' '));
	}

	return args;
}

string ArgumentParser::read_string(const char endchar)
{
	string s;
	while(iterator != end)
	{
		char c = *iterator;
		++iterator;
		if(c == '\\')
		{
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
		if(c == endchar || (endchar == ' ' && c == '\t'))
		{
			return s;
		}
		s += c;
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

}
