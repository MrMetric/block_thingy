#include "PhysicsUtil.hpp"

#include <cstdint>
#include <limits>

#include <glm/common.hpp>		// glm::sign
#include <glm/geometric.hpp>	// glm::normalize
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>		// glm::inverse
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "RaycastHit.hpp"
#include "../Block.hpp"
#include "../World.hpp"
#include "../position/BlockInWorld.hpp"

#include "std_make_unique.hpp"

constexpr double infinity = std::numeric_limits<double>::infinity();

// like fmod(a, 1), but always returns a positive number
// equivalent to a - floor(a)
// which is a - floor(a / 1) * 1
// but std::floor is not constexpr, so it can not be used here
constexpr double mod1(const double a)
{
	return a - (static_cast<int_fast64_t>(a) - ((a < 0) ? 1 : 0));
}

constexpr double intbound(double s, const double ds)
{
	if(ds == 0)
	{
		return infinity;
	}

	// Find the smallest positive t such that s+t*ds is an integer.
	if(ds < 0)
	{
		return intbound(-s, -ds);
	}

	s = mod1(s);
	// problem is now s+t*ds = 1
	return (1 - s) / ds;
}

static glm::dvec3 intbound(const glm::dvec3& origin, const glm::dvec3& direction)
{
	return { intbound(origin.x, direction.x), intbound(origin.y, direction.y), intbound(origin.z, direction.z) };
}

static constexpr double delta(const double x)
{
	// TODO: profile vs (1.0 / glm::abs(x))
	return (x == 0) ? infinity : (glm::sign(x) / x);
}

static glm::dvec3 delta(const glm::dvec3& direction)
{
	return { delta(direction.x), delta(direction.y), delta(direction.z) };
}

void PhysicsUtil::ScreenPosToWorldRay(
	const glm::dvec2& mouse,              // Mouse position, in pixels, from bottom-left corner of the window
	const window_size_t& screen_size,     // Window size, in pixels
	const glm::dmat4& view_matrix,        // Camera position and orientation
	const glm::dmat4& projection_matrix,  // Camera parameters (ratio, field of view, near and far planes)
	glm::dvec3& out_origin,               // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
	glm::dvec3& out_direction             // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
)
{
	// The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
	glm::dvec4 lRayStart_NDC(
		(mouse.x / screen_size.x - 0.5) * 2, // [0,1024] -> [-1,1]
		(mouse.y / screen_size.y - 0.5) * 2, // [0, 768] -> [-1,1]
		-1, // The near plane maps to Z=-1 in Normalized Device Coordinates
		1
	);
	glm::dvec4 lRayEnd_NDC(
		(mouse.x / screen_size.x - 0.5) * 2,
		(mouse.y / screen_size.y - 0.5) * 2,
		0,
		1
	);


	/*
	// The Projection matrix goes from Camera Space to NDC.
	// So inverse(projection_matrix) goes from NDC to Camera Space.
	glm::dmat4 InverseProjectionMatrix = glm::inverse(projection_matrix);

	// The View Matrix goes from World Space to Camera Space.
	// So inverse(view_matrix) goes from Camera Space to World Space.
	glm::dmat4 InverseViewMatrix = glm::inverse(view_matrix);

	glm::dvec4 lRayStart_camera  = InverseProjectionMatrix * lRayStart_NDC;    lRayStart_camera /= lRayStart_camera.w;
	glm::dvec4 lRayStart_world   = InverseViewMatrix       * lRayStart_camera; lRayStart_world  /= lRayStart_world .w;
	glm::dvec4 lRayEnd_camera    = InverseProjectionMatrix * lRayEnd_NDC;      lRayEnd_camera   /= lRayEnd_camera  .w;
	glm::dvec4 lRayEnd_world     = InverseViewMatrix       * lRayEnd_camera;   lRayEnd_world    /= lRayEnd_world   .w;
	*/

	// Faster way (just one inverse)
	glm::dmat4 M = glm::inverse(projection_matrix * view_matrix);
	glm::dvec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world /= lRayStart_world.w;
	glm::dvec4 lRayEnd_world   = M * lRayEnd_NDC;   lRayEnd_world   /= lRayEnd_world.w;


	glm::dvec3 lRayDir_world(lRayEnd_world - lRayStart_world);
	lRayDir_world = glm::normalize(lRayDir_world);


	out_origin = glm::dvec3(lRayStart_world);
	out_direction = lRayDir_world;
}

static bool pos_in_bounds(const Position::BlockInWorld& pos, const glm::dvec3& min, const glm::dvec3& max)
{
	return !(pos.x < min.x || pos.y < min.y || pos.z < min.z || pos.x > max.x || pos.y > max.y || pos.z > max.z);
}

// this stuff is from http://gamedev.stackexchange.com/a/49423
// TODO: fix infinity/NaN stuff
/**
 * Call the callback with (x,y,z,value,face) of all blocks along the line
 * segment from point 'origin' in vector direction 'direction' of length
 * 'radius'. 'radius' may be infinite.
 *
 * 'face' is the normal vector of the face of that block that was entered.
 * It should not be used after the callback returns.
 *
 * If the callback returns a true value, the traversal will be stopped.
 */
std::unique_ptr<RaycastHit> PhysicsUtil::raycast(const World& world, const glm::dvec3& origin, const glm::dvec3& direction, const double radius)
{
	// From "A Fast Voxel Traversal Algorithm for Ray Tracing"
	// by John Amanatides and Andrew Woo, 1987
	// <http://www.cse.yorku.ca/~amana/research/grid.pdf>
	// <http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.42.3443>
	// Extensions to the described algorithm:
	//   • Imposed a distance limit.
	//   • The face passed through to reach the current cube is provided to
	//     the callback.

	// The foundation of this algorithm is a parameterized representation of
	// the provided ray,
	//                    origin + t * direction,
	// except that t is not actually stored; rather, at any given point in the
	// traversal, we keep track of the *greater* t values which we would have
	// if we took a step sufficient to cross a cube boundary along that axis
	// (i.e. change the integer part of the coordinate) in the variables
	// tMaxX, tMaxY, and tMaxZ.

	// Avoids an infinite loop
	if(direction.x == 0 && direction.y == 0 && direction.z == 0)
	{
		return nullptr;
	}

	Position::BlockInWorld cube_pos(origin);

	// Direction to increment x,y,z when stepping.
	const glm::ivec3 step = glm::sign(direction);

	// See description above. The initial values depend on the fractional part of the origin.
	glm::dvec3 tMax = intbound(origin, direction);

	// The change in t when taking a step (always positive).
	const glm::dvec3 tDelta = delta(direction);

	glm::ivec3 face;

	const glm::dvec3 min = origin - radius;
	const glm::dvec3 max = origin + radius;

	while(/* ray has not gone past bounds of world */
			(step.x > 0 ? cube_pos.x < max.x : cube_pos.x > min.x) &&
			(step.y > 0 ? cube_pos.y < max.y : cube_pos.y > min.y) &&
			(step.z > 0 ? cube_pos.z < max.z : cube_pos.z > min.z))
	{
		if(pos_in_bounds(cube_pos, min, max))
		{
			if(world.get_block_const(cube_pos).is_solid())
			{
				return std::make_unique<RaycastHit>(cube_pos, face);
			}
		}

		// tMax.x stores the t-value at which we cross a cube boundary along the
		// X axis, and similarly for Y and Z. Therefore, choosing the least tMax
		// chooses the closest cube boundary.
		uint_fast8_t i = (tMax.x < tMax.y) ? 0 : 1;
		if(tMax.z < tMax[i])
		{
			i = 2;
		}
		cube_pos[i] += step[i];
		// Adjust tMaxX to the next X-oriented boundary crossing.
		tMax[i] += tDelta[i];
		// Record the normal vector of the cube face we entered.
		face.x = face.y = face.z = 0;
		face[i] = -step[i];
	}

	// there is no cube in range
	return nullptr;
}
