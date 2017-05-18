#include "Type.hpp"

#include <ostream>

namespace Block::Enum {

std::ostream& operator<<(std::ostream& os, const Type type)
{
	return os << static_cast<Type_t>(type);
}

}
