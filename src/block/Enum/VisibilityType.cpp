#include "VisibilityType.hpp"

#include <cassert>
#include <ostream>

namespace block_thingy::block::enums {

std::ostream& operator<<(std::ostream& o, const VisibilityType t)
{
	switch(t)
	{
		case VisibilityType::opaque		: return o << "opaque";
		case VisibilityType::translucent: return o << "translucent";
		case VisibilityType::invisible	: return o << "invisible";
	}
	assert(false);
	return o << "ERROR";
}

}
