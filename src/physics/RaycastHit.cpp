#include "RaycastHit.hpp"

#include "block/Enum/Face.hpp"

namespace block_thingy::physics {

using block::enums::Face;

raycast_hit::raycast_hit
(
	const position::block_in_world& pos,
	const glm::ivec3& face
)
:
	pos(pos),
	face_vec(face)
{
}

position::block_in_world raycast_hit::adjacent() const
{
	return pos + face_vec;
}

Face raycast_hit::face() const
{
	if(face_vec.x == 0 && face_vec.y == 0 && face_vec.z == 0) // happens when the camera is in a block
	{
		return Face::front;
	}
	return block::enums::vec_to_face(face_vec);
}

}
