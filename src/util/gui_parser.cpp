#include "gui_parser.hpp"

#include <cctype>
#include <cstddef>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "util/logger.hpp"
#include "util/misc.hpp"

using std::string;

namespace block_thingy::util {

class gui_parser
{
public:
	gui_parser(string name, const string& text);

	json read_thing(std::size_t indent_level);
	json read_primitive_thing(const string&);
	json read_object(std::size_t indent_level);
	json read_string_list(std::size_t indent_level);

private:
	string name;
	std::vector<string> lines;
	std::size_t line_number;
};

json parse_gui(const fs::path& gui_path)
{
	return gui_parser(gui_path.u8string(), read_text(gui_path)).read_object(0);
}

json parse_gui(const string& gui_text)
{
	return gui_parser("text", gui_text).read_object(0);
}

struct operator_info
{
	std::size_t precedence;
	bool is_left_associative;
};
static const std::unordered_map<string, operator_info> OPERATORS
{
	{"=", {0, true}},
	{"+", {1, true}},
	{"-", {1, true}},
	{"*", {2, true}},
	{"/", {2, true}},
};
std::vector<string> parse_expression(const string& expr)
{
	std::vector<string> tokens;

	// TODO: add relational operators > < >= <=
	bool has_equals = false;
	for(auto i = expr.cbegin(); i != expr.cend();)
	{
		char c = *i;
		if(c == ' ' || c == '\t')
		{
			++i;
			continue;
		}
		if(c == '=' || c == '+' || c == '-' || c == '*' || c == '/'
		|| c == '(' || c == ')')
		{
			if(c == '=')
			{
				if(has_equals)
				{
					throw std::runtime_error("expression has multiple equals signs: " + expr);
				}
				has_equals = true;
			}
			tokens.emplace_back(1, c);
			++i;
			continue;
		}
		if(!std::isalnum(c) && c != '_' && c != '$')
		{
			std::ostringstream ss;
			ss << "unexpected character " << c << " at position " << (i - expr.cbegin())
			<< " in expression " << expr;
			throw std::runtime_error(ss.str());
		}
		string token(1, c);
		++i;
		for(; i != expr.cend(); ++i)
		{
			c = *i;
			if(!std::isalnum(c) && c != '_' && c != '.')
			{
				break;
			}
			token += c;
		}
		tokens.emplace_back(std::move(token));
	}
	if(!has_equals)
	{
		throw std::runtime_error("expression has no equals sign: " + expr);
	}

	std::vector<string> rpn;
	std::stack<string> opstack;
	for(const string& token : tokens)
	{
		if(const auto i_op = OPERATORS.find(token); i_op != OPERATORS.cend())
		{
			const std::size_t precedence = i_op->second.precedence;
			while(!opstack.empty())
			{
				const string& op2 = opstack.top();
				if(op2 == "(" || op2 == ")")
				{
					break;
				}
				const auto& op2_info = OPERATORS.at(op2);
				const std::size_t precedence2 = op2_info.precedence;
				if(precedence2 > precedence || precedence2 == precedence && op2_info.is_left_associative)
				{
					rpn.emplace_back(op2); opstack.pop();
				}
				else
				{
					break;
				}
			}
			opstack.emplace(token);
		}
		else if(token == "(")
		{
			opstack.emplace(token);
		}
		else if(token == ")")
		{
			while(!opstack.empty() && opstack.top() != "(")
			{
				rpn.emplace_back(opstack.top()); opstack.pop();
			}
			if(opstack.empty())
			{
				throw std::runtime_error("unmatched right paren in " + expr);
			}
			opstack.pop();
		}
		else // number or var
		{
			rpn.emplace_back(token);
		}
	}
	while(!opstack.empty())
	{
		const string top = opstack.top(); opstack.pop();
		if(top == "(")
		{
			throw std::runtime_error("unmatched left paren in " + expr);
		}
		if(top == ")")
		{
			throw std::runtime_error("unmatched right paren in " + expr);
		}
		rpn.emplace_back(top);
	}
	if(rpn.back() != "=")
	{
		throw std::runtime_error("expression is not an equality: " + expr);
	}

	// TODO: validate expression

	return rpn;
}

gui_parser::gui_parser(string name, const string& text)
:
	name(std::move(name)),
	line_number(0)
{
	std::istringstream input(text);
	for(string line; std::getline(input, line);)
	{
		lines.emplace_back(line);
	}
}

static std::size_t get_indent_level(const string& line)
{
	const std::size_t i = line.find_first_not_of('\t');
	if(i == string::npos)
	{
		return line.size();
	}
	return i;
}

json gui_parser::read_thing(const std::size_t indent_level)
{
	if(line_number >= lines.size())
	{
		return {};
	}
	const string& line = lines[line_number];
	if(line.empty() || line[0] == '#')
	{
		++line_number;
		return read_thing(indent_level);
	}
	const std::size_t line_indent_level = get_indent_level(line);
	if(line_indent_level < indent_level)
	{
		return {};
	}
	++line_number;
	const string text = util::strip_whitespace(line.substr(line_indent_level));
	if(text.empty() || text[0] == '#')
	{
		return read_thing(indent_level);
	}

	if(const json o = read_primitive_thing(text); !o.is_null())
	{
		return o;
	}

	if(text.back() == ':')
	{
		const string key = text.substr(0, text.size() - 1);
		if(key == "widgets")
		{
			json widgets;

			json widget;
			// TODO: the line number is wrong where there is a comment before the widget
			std::size_t widget_line_number = line_number;
			while(!(widget = read_thing(indent_level + 1)).is_null())
			{
				if(!widget.is_object())
				{
					throw std::runtime_error(name + ':' + std::to_string(widget_line_number) + ": invalid widget");
				}
				const string type = widget.cbegin().key();
				widget = widget.cbegin().value();
				if(!widget.is_object())
				{
					throw std::runtime_error(name + ':' + std::to_string(widget_line_number) + ": invalid widget");
				}
				widget.emplace("type", type);
				widgets.emplace_back(std::move(widget));
				widget_line_number = line_number + 1;
			}

			return {{key, widgets}};
		}
		if(key == "layout" || key == "child_layout")
		{
			json layout = read_string_list(indent_level + 1);
			for(json& expr : layout)
			{
				expr = parse_expression(expr);
			}
			return {{key, layout}};
		}
		const json value = read_object(indent_level + 1);
		return {{key, value}};
	}

	if(const std::size_t i = text.find(':'); i != string::npos)
	{
		const string key = util::strip_whitespace(text.substr(0, i));
		const string value_str = util::strip_whitespace(text.substr(i + 1));
		const json value = read_primitive_thing(value_str);
		if(value.is_null())
		{
			throw std::runtime_error(name + ':' + std::to_string(line_number) + ": unable to parse primitive: " + value_str);
		}
		return {{key, value}};
	}

	throw std::runtime_error(name + ':' + std::to_string(line_number) + ": unable to parse " + text);
}

json gui_parser::read_primitive_thing(const string& text)
{
	if(text == "true")
	{
		return true;
	}
	if(text == "false")
	{
		return false;
	}

	try
	{
		return std::stod(text);
	}
	catch(const std::invalid_argument&)
	{
	}
	catch(const std::out_of_range&)
	{
		throw std::runtime_error(name + ':' + std::to_string(line_number) + ": number out of range: " + text);
	}

	if(text.size() > 1 && (text[0] == '"' || text[0] == '\'') && text.back() == text[0])
	{
		return text.substr(1, text.size() - 2);
	}

	return {};
}

json gui_parser::read_object(const std::size_t indent_level)
{
	const json first_thing = read_thing(indent_level);
	if(first_thing.is_null())
	{
		return {};
	}

	if(!first_thing.is_object())
	{
		json list = json::array();
		list.emplace_back(first_thing);
		json thing;
		while(!(thing = read_thing(indent_level)).is_null())
		{
			list.emplace_back(std::move(thing));
		}
		return list;
	}

	json map = first_thing;
	json thing;
	while(!(thing = read_thing(indent_level)).is_null())
	{
		if(!thing.is_object())
		{
			// TODO: line number might be wrong
			throw std::runtime_error(name + ':' + std::to_string(line_number + 1) + ": non-object in map");
		}
		assert(thing.size() == 1); // TODO: determine if this can fail
		const json::const_iterator i = thing.cbegin();
		map.emplace(i.key(), i.value());
	}
	return map;
}

json gui_parser::read_string_list(const std::size_t indent_level)
{
	json list = json::array();
	while(line_number != lines.size())
	{
		const string& line = lines[line_number];
		if(line.empty() || line[0] == '#')
		{
			++line_number;
			continue;
		}
		const std::size_t line_indent_level = get_indent_level(line);
		if(line_indent_level < indent_level)
		{
			break;
		}
		++line_number;
		const string text = line.substr(line_indent_level);
		if(text.empty() || text[0] == '#')
		{
			continue;
		}
		list.emplace_back(text);
	}
	return list;
}

}
