#pragma once

#include <glm/vec3.hpp>

namespace block_thingy::physics {

struct ray
{
	ray(const glm::dvec3& origin, const glm::dvec3& direction);

	glm::dvec3 origin;
	glm::dvec3 direction;
};

}
