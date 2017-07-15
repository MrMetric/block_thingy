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
	if(face_vec.x == 0 && face_vec.y == 0 && face_vec.z == 0) // happens when the camera is in a block
	{
		return Face::front;
	}
	return Block::Enum::vec_to_face(face_vec);
}
