#include "escape_sequence.hpp"

#include <cassert>
#include <cctype>
#include <vector>

#include "util/unicode.hpp"

using std::nullopt;
using std::string;

namespace block_thingy::util {

template<typename T>
static std::optional<T> get_oct(const char c)
{
	if(c >= '0' && c <= '7')
	{
		return static_cast<T>(c - '0');
	}
	return {};
}

template<typename T>
static std::optional<T> get_hex(const char c)
{
	if(c >= '0' && c <= '9')
	{
		return static_cast<T>(c - '0');
	}
	if(c >= 'A' && c <= 'F')
	{
		return static_cast<T>(c - 'A' + 10);
	}
	if(c >= 'a' && c <= 'f')
	{
		return static_cast<T>(c - 'a' + 10);
	}
	return {};
}

std::optional<string> parse_escape_sequence
(
	string::const_iterator& i_,
	const string::const_iterator end
)
{
	if(i_ >= end)
	{
		return {};
	}

	string::const_iterator i = i_;
	string s;
	s += *i++;
	if(s[0] >= '0' && s[0] <= '7'  // octal
	|| s[0] == 'x' || s[0] == 'X') // hexadecimal
	{
		if((end - i) < 2)
		{
			return {};
		}
		s += *i++;
		s += *i++;
	}
	else if(s[0] == 'u' || s[0] == 'U') // unicode
	{
		while(s.size() != 9) // up to 8 digits
		{
			if(i == end)
			{
				break;
			}
			const char c = *i;
			if(!std::isxdigit(c))
			{
				break;
			}
			s += c;
			++i;
		}
		if(s.size() == 1) // no digits added
		{
			return {};
		}
	}
	const auto ret = parse_escape_sequence(s);
	if(ret != nullopt)
	{
		i_ = i;
	}
	return ret;
}

std::optional<string> parse_escape_sequence(const char c)
{
	return parse_escape_sequence(string(1, c));
}

// http://en.cppreference.com/w/cpp/language/escape
std::optional<string> parse_escape_sequence(const string& seq)
{
	if(seq.empty())
	{
		return {};
	}

	if(seq.size() == 1)
	{
		switch(seq[0])
		{
			case '0' : return string(1, '\0');
			case 'a' : return string(1, '\a'); // audible bell
			case 'b' : return string(1, '\b'); // backspace
			case 'e' : return string(1, '\033');
			case 'f' : return string(1, '\f'); // form feed (new page)
			case 'n' : return string(1, '\n');
			case 'r' : return string(1, '\r');
			case 't' : return string(1, '\t');
			case 'v' : return string(1, '\v'); // vertical tab
			case '"' : return string(1, '"');
			case '\'': return string(1, '\'');
			case '\\': return string(1, '\\');
		}
		return {};
	}

	if(seq[0] == 'u' || seq[0] == 'U')
	{
		assert(seq.size() != 1);

		if(seq.size() > 9) // up to 8 digits
		{
			return {};
		}

		char32_t c = 0;
		char32_t m = 1;
		for(auto i = seq.crbegin(); i != seq.crend() - 1; ++i)
		{
			const std::optional<char32_t> d = get_hex<char32_t>(*i);
			if(d == nullopt)
			{
				return {};
			}
			c += *d * m;
			m *= 0x10;
		}
		try
		{
			return utf32_to_utf8(c);
		}
		catch(...)
		{
			return "\uFFFD";
		}
	}

	if(seq.size() == 3)
	{
		if(seq[0] == 'x' || seq[0] == 'X')
		{
			const std::optional<char> d1 = get_hex<char>(seq[1]);
			const std::optional<char> d0 = get_hex<char>(seq[2]);
			if(d1 == nullopt || d0 == nullopt)
			{
				return {};
			}
			const char c = static_cast<char>(*d1 * 0x10 + *d0);
			return string(1, c);
		}

		const std::optional<char> d2 = get_oct<char>(seq[0]);
		const std::optional<char> d1 = get_oct<char>(seq[1]);
		const std::optional<char> d0 = get_oct<char>(seq[2]);
		if(d2 == nullopt || d1 == nullopt || d0 == nullopt)
		{
			return {};
		}
		const char c = static_cast<char>(*d2 * 0100 + *d1 * 0010 + *d0);
		return string(1, c);
	}

	return {};
}

}
