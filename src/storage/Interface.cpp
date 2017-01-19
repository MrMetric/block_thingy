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
