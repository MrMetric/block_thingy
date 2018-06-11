#include "Face.hpp"

#include <cassert>
#include <ostream>
#include <string>
#include <type_traits>

namespace block_thingy::block::enums {

glm::ivec3 face_to_vec(const Face face)
{
	switch(face)
	{
		case Face::right : return {+1,  0,  0};
		case Face::left  : return {-1,  0,  0};
		case Face::top   : return { 0, +1,  0};
		case Face::bottom: return { 0, -1,  0};
		case Face::front : return { 0,  0, +1};
		case Face::back  : return { 0,  0, -1};
	}
	assert(false);
	// to satisfy -Werror
	return {};
}

Face vec_to_face(const glm::ivec3& vec)
{
	if(vec.x == +1) return Face::right;
	if(vec.x == -1) return Face::left;
	if(vec.y == +1) return Face::top;
	if(vec.y == -1) return Face::bottom;
	if(vec.z == +1) return Face::front;
	if(vec.z == -1) return Face::back;
	assert(false);
	// to satisfy -Werror
	return Face::front;
}

std::ostream& operator<<(std::ostream& o, const Face face)
{
	switch(face)
	{
		case Face::front : return o << "front";
		case Face::back  : return o << "back";
		case Face::top   : return o << "top";
		case Face::bottom: return o << "bottom";
		case Face::right : return o << "right";
		case Face::left  : return o << "left";
	}
	assert(false);
	// to satisfy -Werror
	const auto i = static_cast<std::underlying_type_t<Face>>(face);
	return o << "ERROR(" << std::to_string(i) << ')';
}

}
