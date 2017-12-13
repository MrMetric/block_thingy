#pragma once
#include "frustum.hpp"

namespace Graphics {

template<bool B>
class null_frustum : public frustum
{
public:
	bool inside(const glm::dvec3&) const override
	{
		return B;
	}

	bool inside(const Physics::AABB&) const override
	{
		return B;
	}
};

}
