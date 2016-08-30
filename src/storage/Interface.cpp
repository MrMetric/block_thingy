#include "Interface.hpp"

namespace msgpack {
namespace v1 {

template<>
template<>
packer<Poco::DeflatingOutputStream>& packer<Poco::DeflatingOutputStream>::pack(const msgpack::sbuffer& v)
{
	append_buffer(v.data(), v.size());
	return *this;
}

} // namespace v1
} // namespace msgpack
