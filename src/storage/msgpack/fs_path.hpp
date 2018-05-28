#pragma once

#include <string>

#include "util/filesystem.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<>
struct pack<fs::path>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const fs::path& v) const
	{
		o.pack(v.generic_u8string());
		return o;
	}
};

template<>
struct convert<fs::path>
{
	const msgpack::object& operator()(const msgpack::object& o, fs::path& v) const
	{
		if(o.type != msgpack::type::STR) throw msgpack::type_error();
		v = o.as<std::string>(); // I dunno if this behaves on Windows
		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
