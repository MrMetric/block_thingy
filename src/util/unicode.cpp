#include "unicode.hpp"

#include <codecvt>
#include <locale>

using std::string;
using std::u32string;

namespace Util {

#if defined(_MSC_VER) && _MSC_VER >= 1900 // a bug prevents linking
#define bugfix
static std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> convert;
#else
static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
#endif

u32string utf8_to_utf32(const string& s)
{
	#ifdef bugfix
	const std::basic_string<int32_t> s2 = convert.from_bytes(s);
	return u32string(reinterpret_cast<const char32_t*>(s2.data()), s2.size());
	#else
	return convert.from_bytes(s);
	#endif
}

string utf32_to_utf8(const u32string& s)
{
	#ifdef bugfix
	std::basic_string<int32_t> s2(reinterpret_cast<const int32_t*>(s.data()), s.size());
	return convert.to_bytes(s2);
	#else
	return convert.to_bytes(s);
	#endif
}

} // namespace Util
