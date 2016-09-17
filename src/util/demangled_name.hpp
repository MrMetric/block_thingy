#pragma once

#include <string>
#include <typeinfo>

namespace Util {

std::string demangle(const std::string&);

template<typename T>
std::string demangled_name(const T& o)
{
	return demangle(typeid(o).name());
}

} // namespace Util
