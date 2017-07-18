#pragma once

#include <stdint.h>
#include <unordered_map>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "block/Enum/Face.hpp"
#include "position/hash.hpp"

namespace Block::RotationUtil {

using uvec3 = glm::tvec3<uint8_t>;
using ivec3 = glm::tvec3<int8_t>;
using imat4 = glm::mat<4, 4, int8_t>;

extern std::unordered_map<uvec3, std::unordered_map<Enum::Face, uint8_t>, Position::hasher_struct<uvec3>> face_rotation_LUT;
imat4 rotate(uint8_t turns, const ivec3& axis);
Enum::Face rotate_face(const Enum::Face, const uvec3&);
uvec3 mat_to_rot(const imat4&);

}
