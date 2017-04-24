#include "Face.hpp"

#include <cassert>
#include <ostream>

namespace Block::Enum {

std::ostream& operator<<(std::ostream& os, const Face face)
{
	switch(face)
	{
		case Face::front : return os << "front";
		case Face::back  : return os << "back";
		case Face::top   : return os << "top";
		case Face::bottom: return os << "bottom";
		case Face::right : return os << "right";
		case Face::left  : return os << "left";
	}
	assert(false);
	// to satisfy -Werror
	return os << "ERROR";
}

}
