#include "plane.hpp"

#include <glm/geometric.hpp>

#include "physics/AABB.hpp"

namespace block_thingy::graphics {

plane::plane()
:
	d(0)
{
}

plane::plane(const glm::dvec3& a, const glm::dvec3& b, const glm::dvec3& c)
:
	normal(glm::normalize(glm::cross(c - b, a - b))),
	d(-glm::dot(normal, b))
{
}

double plane::distance(const glm::dvec3& x) const
{
	return d + glm::dot(normal, x);
}

double plane::distance_p(const physics::AABB& aabb) const
{
	return distance
	({
		(normal.x > 0) ? aabb.max.x : aabb.min.x,
		(normal.y > 0) ? aabb.max.y : aabb.min.y,
		(normal.z > 0) ? aabb.max.z : aabb.min.z,
	});
}

double plane::distance_n(const physics::AABB& aabb) const
{
	return distance
	({
		(normal.x < 0) ? aabb.max.x : aabb.min.x,
		(normal.y < 0) ? aabb.max.y : aabb.min.y,
		(normal.z < 0) ? aabb.max.z : aabb.min.z,
	});
}

}
