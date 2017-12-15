#include "ray.hpp"

namespace block_thingy::physics {

ray::ray(const glm::dvec3& origin, const glm::dvec3& direction)
:
	origin(origin),
	direction(direction)
{
}

}
