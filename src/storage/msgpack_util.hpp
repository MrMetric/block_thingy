#pragma once

#include <map>
#include <string>
#include <typeinfo>

#include <msgpack.hpp>

#include "util/demangled_name.hpp"
#include "util/logger.hpp"
#include "util/Property.hpp"

namespace block_thingy::storage {

template<typename T>
bool find_in_map
(
	const std::map<std::string, msgpack::object>& map,
	const std::string& key,
	T& v
)
{
	const auto i = map.find(key);
	if(i == map.cend())
	{
		return false;
	}
	v = i->second.as<T>();
	return true;
}

template<typename T>
bool find_in_map
(
	const std::map<std::string, msgpack::object>& map,
	const std::string& key,
	util::property<T>& p
)
{
	T v;
	const bool a = find_in_map(map, key, v);
	if(a)
	{
		p = v;
	}
	return a;
}

template<typename T>
void find_in_map_or_throw
(
	const std::map<std::string, msgpack::object>& map,
	const std::string& key,
	T& v
)
{
	if(!find_in_map(map, key, v))
	{
		// TODO: put message string in type_error
		LOG(ERROR) << "did not find '" << key << "' of type " << util::demangled_name(v) << '\n';
		throw msgpack::type_error();
	}
}

template<typename T>
void unpack_bytes(const std::string& bytes, T& v)
{
	msgpack::unpacked u;
	msgpack::unpack(u, bytes.data(), bytes.length());
	msgpack::object o = u.get();
	o.convert(v);
}

}
