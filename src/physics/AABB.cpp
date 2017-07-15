#include "AABB.hpp"

#include "position/BlockInWorld.hpp"

namespace Physics {

AABB::AABB()
{
}

AABB::AABB(const glm::dvec3& min, const glm::dvec3& max)
:
	min(min),
	max(max)
{
}

AABB::AABB(const Position::BlockInWorld& block_pos)
:
	min(block_pos.x, block_pos.y, block_pos.z),
	max(min + 1.0)
{
}

bool AABB::collide(const AABB& b2)
{
	const AABB& b1 = *this;
	//return b1.max > b2.min && b1.min < b2.max;
	return
	b1.max.x > b2.min.x &&
	b1.max.y > b2.min.y &&
	b1.max.z > b2.min.z &&
	b1.min.x < b2.max.x &&
	b1.min.y < b2.max.y &&
	b1.min.z < b2.max.z;
}

glm::dvec3 AABB::offset(const AABB& b2, const glm::vec3& direction_sign)
{
	const AABB& b1 = *this;
	glm::dvec3 offset;
	auto set = [&b1, &b2, &direction_sign, &offset](glm::dvec3::length_type i)
	{
		if(direction_sign[i] < 0)
		{
			offset[i] = b2.max[i] - b1.min[i];
		}
		else if(direction_sign[i] > 0)
		{
			offset[i] = b2.min[i] - b1.max[i];
		}
	};
	set(0);
	set(1);
	set(2);
	return offset;
}

}
