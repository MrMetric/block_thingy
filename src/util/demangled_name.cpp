#include "demangled_name.hpp"

#include <string>

using std::string;

#if defined(__GNUG__) && __has_include(<cxxabi.h>) // TODO: find libc++ equivalent
#include <cstdlib>
#include <cxxabi.h>
#include <memory>

using std::unique_ptr;

string Util::demangle(const string& s)
{
	int status;
	unique_ptr<char, void(*)(void*)> res
	{
		abi::__cxa_demangle(s.c_str(), nullptr, nullptr, &status),
		std::free,
	};
	return (status == 0) ? res.get() : s;
}
#else
string Util::demangle(const string& s)
{
	return s;
}
#endif
