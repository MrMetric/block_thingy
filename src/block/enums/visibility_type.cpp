#include "visibility_type.hpp"

#include <cassert>
#include <ostream>
#include <string>
#include <type_traits>

namespace block_thingy::block::enums {

std::ostream& operator<<(std::ostream& o, const visibility_type t)
{
	switch(t)
	{
		case visibility_type::opaque     : return o << "opaque";
		case visibility_type::translucent: return o << "translucent";
		case visibility_type::invisible  : return o << "invisible";
	}
	assert(false);
	// to satisfy -Werror
	const auto i = static_cast<std::underlying_type_t<visibility_type>>(t);
	return o << "ERROR(" << std::to_string(i) << ')';
}

}
