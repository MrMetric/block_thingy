#pragma once

#include <glm/vec3.hpp>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<typename T>
struct pack<glm::tvec3<T>>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const glm::tvec3<T>& v) const
	{
		o.pack_array(3);
		o.pack(v.x);
		o.pack(v.y);
		o.pack(v.z);
		return o;
	}
};

template<typename T>
struct convert<glm::tvec3<T>>
{
	const msgpack::object& operator()(const msgpack::object& o, glm::tvec3<T>& v) const
	{
		if(o.type != msgpack::type::ARRAY) throw msgpack::type_error();
		if(o.via.array.size != 3) throw msgpack::type_error();

		v.x = o.via.array.ptr[0].as<T>();
		v.y = o.via.array.ptr[1].as<T>();
		v.z = o.via.array.ptr[2].as<T>();

		return o;
	}
};

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
