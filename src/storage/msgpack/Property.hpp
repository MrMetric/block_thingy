#pragma once

#include "util/Property.hpp"

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

using block_thingy::util::property;

template<typename T>
struct pack<property<T>>
{
	template<typename Stream>
	packer<Stream>& operator()(packer<Stream>& o, const property<T>& v) const
	{
		o.pack(v());
		return o;
	}
};

}
}
}
