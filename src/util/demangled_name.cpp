#include "demangled_name.hpp"

#include <cstdlib>
#include <cxxabi.h>
#include <memory>

using std::string;
using std::unique_ptr;

namespace Util {

string demangle(const string& s)
{
	#ifdef __GNUG__
	int status;
	unique_ptr<char, void(*)(void*)> res
	{
		abi::__cxa_demangle(s.c_str(), nullptr, nullptr, &status),
		std::free,
	};
	return (status == 0) ? res.get() : s;
	#else
	return s;
	#endif
}

} // namespace Util
