#include "Face.hpp"

#include <cassert>
#include <ostream>

namespace Block::Enum {

glm::ivec3 face_to_vec(const Face face)
{
	glm::ivec3 face_vec;
	switch(face)
	{
		case Face::right : face_vec.x = +1; break;
		case Face::left  : face_vec.x = -1; break;
		case Face::top   : face_vec.y = +1; break;
		case Face::bottom: face_vec.y = -1; break;
		case Face::front : face_vec.z = +1; break;
		case Face::back  : face_vec.z = -1; break;
	}
	assert(false);
	return face_vec; // to satisfy -Werror
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
	return Face::front; // to satisfy -Werror
}

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
	return os << "ERROR"; // to satisfy -Werror
}

}
