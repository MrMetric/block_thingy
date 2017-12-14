#pragma once

#include <glm/vec3.hpp>

#include "fwd/physics/AABB.hpp"

namespace block_thingy::graphics {

class frustum
{
public:
	virtual ~frustum();

	virtual bool inside(const glm::dvec3&) const = 0;
	virtual bool inside(const physics::AABB&) const = 0;
};

}
