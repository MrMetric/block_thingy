#include "Type.hpp"

#include <ostream>

namespace Block::Enum {

std::ostream& operator<<(std::ostream& o, const Type type)
{
	return o << static_cast<Type_t>(type);
}

std::ostream& operator<<(std::ostream& o, const TypeExternal type)
{
	return o << static_cast<Type_t>(type);
}

}
