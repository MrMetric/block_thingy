#pragma once

#include <glm/vec3.hpp>

#include "fwd/physics/AABB.hpp"

namespace Graphics {

class frustum
{
public:
	virtual ~frustum();

	virtual bool inside(const glm::dvec3&) const = 0;
	virtual bool inside(const Physics::AABB&) const = 0;
};

}
