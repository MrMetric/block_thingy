#include "default_view_frustum.hpp"

#include <cmath>
#include <utility>

#include <glm/geometric.hpp>

namespace Graphics {

default_view_frustum::default_view_frustum
(
	const glm::dvec3& camera_position,
	const glm::dvec3& camera_rotation,
	const double near_plane,
	const double far_plane,
	const double fov,
	const double ratio
)
{
	const double sinX = std::sin(camera_rotation.x);
	const double cosX = std::cos(camera_rotation.x);
	const double sinY = std::sin(camera_rotation.y);
	const double cosY = std::cos(camera_rotation.y);
	const glm::dvec3 look = glm::normalize(glm::dvec3(-cosX * sinY, sinX, cosX * cosY));
	const glm::dvec3 right = glm::normalize(glm::cross(look, {0, 1, 0}));
	const glm::dvec3 up = glm::normalize(glm::cross(look, right));

	const double tan_fov = std::tan(fov / 2);

	const double near_height = tan_fov * near_plane;
	const double near_width = near_height * ratio;
	const glm::dvec3 near_center = camera_position - look * near_plane;
	const glm::dvec3 near_up = up * near_height;
	const glm::dvec3 near_right = right * near_width;
	const glm::dvec3 ntl = near_center + near_up - near_right;
	const glm::dvec3 ntr = near_center + near_up + near_right;
	const glm::dvec3 nbl = near_center - near_up - near_right;
	const glm::dvec3 nbr = near_center - near_up + near_right;

	const double far_height = tan_fov * far_plane;
	const double far_width = far_height * ratio;
	const glm::dvec3 far_center = camera_position - look * far_plane;
	const glm::dvec3 far_up = up * far_height;
	const glm::dvec3 far_right = right * far_width;
	const glm::dvec3 ftl = far_center + far_up - far_right;
	const glm::dvec3 ftr = far_center + far_up + far_right;
	const glm::dvec3 fbl = far_center - far_up - far_right;
	const glm::dvec3 fbr = far_center - far_up + far_right;

	planes =
	{
		{ntr, ntl, ftl}, // top
		{nbl, nbr, fbr}, // bottom
		{ntl, nbl, fbl}, // left
		{nbr, ntr, fbr}, // right
		{ntl, ntr, nbr}, // near
		{ftr, ftl, fbl}, // far
	};
}

default_view_frustum::default_view_frustum
(
	const glm::dvec3& camera_position,
	const glm::dvec3& camera_rotation,
	const double near_plane,
	const double fov,
	const double ratio
)
:
	default_view_frustum(camera_position, camera_rotation, near_plane, near_plane + 1, fov, ratio)
{
	planes.pop_back();
}

default_view_frustum::default_view_frustum(default_view_frustum&& that)
:
	planes(std::move(that.planes))
{
}

default_view_frustum::default_view_frustum(const default_view_frustum& that)
:
	planes(that.planes)
{
}

default_view_frustum& default_view_frustum::operator=(default_view_frustum&& that)
{
	planes = std::move(that.planes);
	return *this;
}

default_view_frustum& default_view_frustum::operator=(const default_view_frustum& that)
{
	planes = that.planes;
	return *this;
}

bool default_view_frustum::inside(const glm::dvec3& point) const
{
	for(const plane& p : planes)
	{
		if(p.distance(point) < 0)
		{
			return false;
		}
	}
	return true;
}

bool default_view_frustum::inside(const Physics::AABB& aabb) const
{
	for(const plane& p : planes)
	{
		if(p.distance_p(aabb) < 0)
		{
			return false;
		}
	}
	return true;
}

}
