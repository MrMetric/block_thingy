#include "VisibilityType.hpp"

#include <cassert>
#include <ostream>

namespace block_thingy::block::enums {

std::ostream& operator<<(std::ostream& o, const visibility_type t)
{
	switch(t)
	{
		case visibility_type::opaque		: return o << "opaque";
		case visibility_type::translucent: return o << "translucent";
		case visibility_type::invisible	: return o << "invisible";
	}
	assert(false);
	return o << "ERROR";
}

}
