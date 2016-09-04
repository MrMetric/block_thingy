#include "demangled_name.hpp"

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#include <memory>

using std::string;
using std::unique_ptr;

namespace Util {

string demangle(const string& s)
{
	int status;
	unique_ptr<char, void(*)(void*)> res
	{
		abi::__cxa_demangle(s.c_str(), nullptr, nullptr, &status),
		std::free,
	};
	return (status == 0) ? res.get() : s;
}

} // namespace Util
#else
std::string Util::demangle(const std::string& s)
{
	return s;
}
#endif
