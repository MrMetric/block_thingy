#include "RaycastHit.hpp"

#include <cassert>

#include "block/Enum/Face.hpp"
#include "position/BlockInWorld.hpp"

using Block::Enum::Face;

RaycastHit::RaycastHit
(
	const Position::BlockInWorld& pos,
	const glm::ivec3& face
)
:
	pos(pos),
	face_vec(face)
{
}

Position::BlockInWorld RaycastHit::adjacent() const
{
	return pos + face_vec;
}

Face RaycastHit::face() const
{
	if(face_vec.x == +1) return Face::left;
	if(face_vec.x == -1) return Face::right;
	if(face_vec.y == +1) return Face::top;
	if(face_vec.y == -1) return Face::bottom;
	if(face_vec.z == +1) return Face::back;
	if(face_vec.z == -1) return Face::front;
	// happens when the camera is in a block
	assert(face_vec.x == 0 && face_vec.y == 0 && face_vec.z == 0);
	return Face::front;
}
