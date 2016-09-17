#pragma once

#include "util/Property.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

template<typename T>
struct pack<Property<T>>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const Property<T>& v) const
	{
		o.pack(v());
		return o;
	}
};

}
}
}
