#include "RaycastHit.hpp"

#include "position/BlockInWorld.hpp"

RaycastHit::RaycastHit
(
	const Position::BlockInWorld& pos,
	const glm::ivec3& face
)
:
	pos(pos),
	face(face)
{
}

Position::BlockInWorld RaycastHit::adjacent() const
{
	return pos + face;
}
