#include "type.hpp"

#include <ostream>

namespace block_thingy::block::enums {

std::ostream& operator<<(std::ostream& o, const type type)
{
	return o << static_cast<type_t>(type);
}

std::ostream& operator<<(std::ostream& o, const type_external type)
{
	return o << static_cast<type_t>(type);
}

}
