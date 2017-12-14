#pragma once

#include <glm/vec3.hpp>

#include "fwd/physics/AABB.hpp"

namespace block_thingy::graphics {

struct plane
{
	plane();
	plane(const glm::dvec3&, const glm::dvec3&, const glm::dvec3&);

	double distance(const glm::dvec3&) const;
	double distance_p(const physics::AABB&) const;
	double distance_n(const physics::AABB&) const;

	glm::dvec3 normal;
	double d;
};

}
