#include "Interface.hpp"

namespace msgpack {
namespace v1 {

template<>
template<>
packer<zstr::ostream>& packer<zstr::ostream>::pack(const msgpack::sbuffer& v)
{
	append_buffer(v.data(), v.size());
	return *this;
}

} // namespace v1
} // namespace msgpack


namespace Storage {

msgpack::object_handle copy_object(const msgpack::object& o)
{
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, o);
	return msgpack::unpack(buffer.data(), buffer.size());
}

std::map<std::string, msgpack::object_handle> InputInterface::copy_all()
{
	std::map<std::string, msgpack::object_handle> copy;
	for(const auto& p : map)
	{
		copy.emplace(p.first, copy_object(p.second));
	}
	return copy;
}

}
