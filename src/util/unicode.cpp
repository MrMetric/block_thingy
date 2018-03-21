#include "unicode.hpp"

#include <cassert>
#include <codecvt>
#include <locale>

using std::string;
using std::u32string;

namespace block_thingy::util {

#if defined(_MSC_VER) && _MSC_VER >= 1900 // a bug prevents linking
	#define bugfix
	static std::wstring_convert<std::codecvt_utf8<uint32_t>, uint32_t> convert;
#else
	static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
#endif

char32_t utf8_to_utf32(const char c)
{
	const u32string s = utf8_to_utf32(string(1, c));
	assert(s.size() == 1);
	return s[0];
}

u32string utf8_to_utf32(const string& s)
{
#ifdef bugfix
	const std::basic_string<uint32_t> s2 = convert.from_bytes(s);
	return u32string(reinterpret_cast<const char32_t*>(s2.data()), s2.size());
#else
	return convert.from_bytes(s);
#endif
}

string utf32_to_utf8(const char32_t c)
{
	return convert.to_bytes(c);
}

string utf32_to_utf8(const u32string& s)
{
#ifdef bugfix
	std::basic_string<uint32_t> s2(reinterpret_cast<const uint32_t*>(s.data()), s.size());
	return convert.to_bytes(s2);
#else
	return convert.to_bytes(s);
#endif
}

}
