#pragma once
#include "frustum.hpp"

#include <vector>

#include "graphics/plane.hpp"

namespace Graphics {

class default_view_frustum : public frustum
{
public:
	default_view_frustum
	(
		const glm::dvec3& camera_position,
		const glm::dvec3& camera_rotation,
		double near_plane,
		double far_plane,
		double fov,
		double ratio
	);

	// infinite projection; no far plane
	default_view_frustum
	(
		const glm::dvec3& camera_position,
		const glm::dvec3& camera_rotation,
		double near_plane,
		double fov,
		double ratio
	);

	default_view_frustum(default_view_frustum&&);
	default_view_frustum(const default_view_frustum&);
	default_view_frustum& operator=(default_view_frustum&&);
	default_view_frustum& operator=(const default_view_frustum&);

	bool inside(const glm::dvec3&) const override;
	bool inside(const Physics::AABB&) const override;

private:
	std::vector<plane> planes;
};

}
