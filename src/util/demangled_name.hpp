#pragma once

#include <string>
#include <typeinfo>

namespace block_thingy::util {

std::string demangle(const std::string&);

template<typename T>
std::string demangled_name()
{
	return demangle(typeid(T).name());
}

template<typename T>
std::string demangled_name(const T& o)
{
	return demangle(typeid(o).name());
}

}
