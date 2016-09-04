#include "unicode.hpp"

#include <codecvt>
#include <locale>

using std::string;
using std::u32string;

namespace Util {

u32string utf8_to_utf32(const string& s)
{
	#if _MSC_VER == 1900 // a bug prevents linking
	static std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> convert;
	const auto s2 = convert.from_bytes(s);
	return u32string(reinterpret_cast<const char32_t*>(s2.data()), s2.size());
	#else
	static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
	return convert.from_bytes(s);
	#endif
}

} // namespace Util
