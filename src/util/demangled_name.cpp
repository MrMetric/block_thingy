#include "demangled_name.hpp"

#include <string>

using std::string;

#if __has_include(<cxxabi.h>)
// https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a01696.html
#include <cstdlib>
#include <cxxabi.h>
#include <memory>

using std::unique_ptr;

namespace block_thingy::util {

string demangle(const string& s)
{
	int status;
	std::size_t length;
	unique_ptr<char, void(*)(void*)> res
	{
		abi::__cxa_demangle(s.c_str(), nullptr, &length, &status),
		std::free,
	};
	return (status == 0) ? string(res.get(), length) : s;
}
#else
namespace block_thingy::util {

string demangle(const string& s)
{
	return s;
}
#endif

}
