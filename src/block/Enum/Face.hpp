#pragma once

#include <iosfwd>
#include <stdint.h>

#include <glm/vec3.hpp>

namespace Block::Enum {

enum class Face : uint8_t
{
	right  = 0, // +x
	left   = 1, // -x
	top    = 2, // +y
	bottom = 3, // -y
	front  = 4, // +z
	back   = 5, // -z
};

glm::ivec3 face_to_vec(Face);
Face vec_to_face(const glm::ivec3&);

std::ostream& operator<<(std::ostream&, Face);

}
