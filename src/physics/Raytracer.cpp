#include "Raytracer.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include <glm/common.hpp>		// glm::sign
#include <glm/geometric.hpp>	// glm::normalize
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>		// glm::inverse
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "RaytraceHit.hpp"
#include "../Block.hpp"
#include "../Coords.hpp"
#include "../World.hpp"

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
	// Find the smallest positive t such that s+t*ds is an integer.
	if(ds < 0)
	{
		return intbound(-s, -ds);
	}
	if(ds == 0)
	{
		return infinity;
	}

	s = mod1(s);
	// problem is now s+t*ds = 1
	return (1 - s) / ds;
}

void Raytracer::ScreenPosToWorldRay(
	uint_fast32_t mouseX, uint_fast32_t mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
	uint_fast32_t screenWidth, uint_fast32_t screenHeight,  // Window size, in pixels
	glm::dmat4 ViewMatrix,              // Camera position and orientation
	glm::dmat4 ProjectionMatrix,        // Camera parameters (ratio, field of view, near and far planes)
	glm::dvec3& out_origin,             // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
	glm::dvec3& out_direction           // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
)
{
	// The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
	glm::dvec4 lRayStart_NDC(
		(double(mouseX) / screenWidth  - 0.5) * 2, // [0,1024] -> [-1,1]
		(double(mouseY) / screenHeight - 0.5) * 2, // [0, 768] -> [-1,1]
		-1, // The near plane maps to Z=-1 in Normalized Device Coordinates
		1
	);
	glm::dvec4 lRayEnd_NDC(
		(double(mouseX) / screenWidth  - 0.5) * 2,
		(double(mouseY) / screenHeight - 0.5) * 2,
		0,
		1
	);


	// The Projection matrix goes from Camera Space to NDC.
	// So inverse(ProjectionMatrix) goes from NDC to Camera Space.
	glm::dmat4 InverseProjectionMatrix = glm::inverse(ProjectionMatrix);

	// The View Matrix goes from World Space to Camera Space.
	// So inverse(ViewMatrix) goes from Camera Space to World Space.
	glm::dmat4 InverseViewMatrix = glm::inverse(ViewMatrix);

	glm::dvec4 lRayStart_camera = InverseProjectionMatrix * lRayStart_NDC;    lRayStart_camera /= lRayStart_camera.w;
	glm::dvec4 lRayStart_world  = InverseViewMatrix       * lRayStart_camera; lRayStart_world  /= lRayStart_world .w;
	glm::dvec4 lRayEnd_camera   = InverseProjectionMatrix * lRayEnd_NDC;      lRayEnd_camera   /= lRayEnd_camera  .w;
	glm::dvec4 lRayEnd_world    = InverseViewMatrix       * lRayEnd_camera;   lRayEnd_world    /= lRayEnd_world   .w;


	// Faster way (just one inverse)
	//glm::mat4 M = glm::inverse(ProjectionMatrix * ViewMatrix);
	//glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world/=lRayStart_world.w;
	//glm::vec4 lRayEnd_world   = M * lRayEnd_NDC  ; lRayEnd_world  /=lRayEnd_world.w;


	glm::dvec3 lRayDir_world(lRayEnd_world - lRayStart_world);
	lRayDir_world = glm::normalize(lRayDir_world);


	out_origin = glm::dvec3(lRayStart_world);
	out_direction = lRayDir_world;
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
std::unique_ptr<RaytraceHit> Raytracer::raycast(const World& world, glm::dvec3 origin, glm::dvec3 direction, double radius)
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

	// Cube containing origin point.
	BlockInWorld_type x = static_cast<BlockInWorld_type>(std::floor(origin[0]));
	BlockInWorld_type y = static_cast<BlockInWorld_type>(std::floor(origin[1]));
	BlockInWorld_type z = static_cast<BlockInWorld_type>(std::floor(origin[2]));

	double dx = direction[0];
	double dy = direction[1];
	double dz = direction[2];

	// Avoids an infinite loop.
	if(dx == 0 && dy == 0 && dz == 0)
	{
		throw std::invalid_argument("direction must not be (0, 0, 0)");
	}

	// Direction to increment x,y,z when stepping.
	int stepX = int(glm::sign(dx));
	int stepY = int(glm::sign(dy));
	int stepZ = int(glm::sign(dz));

	// See description above. The initial values depend on the fractional part of the origin.
	double tMaxX = intbound(origin[0], dx);
	double tMaxY = intbound(origin[1], dy);
	double tMaxZ = intbound(origin[2], dz);

	// The change in t when taking a step (always positive).
	double tDeltaX = dx == 0 ? infinity : stepX / dx;
	double tDeltaY = dy == 0 ? infinity : stepY / dy;
	double tDeltaZ = dz == 0 ? infinity : stepZ / dz;

	glm::vec3 face;

	double minX = origin.x - radius;
	double minY = origin.y - radius;
	double minZ = origin.z - radius;
	double maxX = origin.x + radius;
	double maxY = origin.y + radius;
	double maxZ = origin.z + radius;

	while(/* ray has not gone past bounds of world */
			(stepX > 0 ? x < maxX : x > minX) &&
			(stepY > 0 ? y < maxY : y > minY) &&
			(stepZ > 0 ? z < maxZ : z > minZ))
	{
		// Invoke the callback, unless we are not *yet* within the bounds of the world.
		if(!(x < minX || y < minY || z < minZ || x > maxX || y > maxY || z > maxZ))
		{
			Position::BlockInWorld pos(x, y, z);
			if(world.get_block_const(pos).is_solid())
			{
				return std::make_unique<RaytraceHit>(pos, face);
			}
		}

		// tMaxX stores the t-value at which we cross a cube boundary along the
		// X axis, and similarly for Y and Z. Therefore, choosing the least tMax
		// chooses the closest cube boundary. Only the first case of the four
		// has been commented in detail.
		if(tMaxX < tMaxY)
		{
			if(tMaxX < tMaxZ)
			{
				// Update which cube we are now in.
				x += stepX;
				// Adjust tMaxX to the next X-oriented boundary crossing.
				tMaxX += tDeltaX;
				// Record the normal vector of the cube face we entered.
				face[0] = -stepX;
				face[1] = 0;
				face[2] = 0;
			}
			else
			{
				z += stepZ;
				tMaxZ += tDeltaZ;
				face[0] = 0;
				face[1] = 0;
				face[2] = -stepZ;
			}
		}
		else
		{
			if(tMaxY < tMaxZ)
			{
				y += stepY;
				tMaxY += tDeltaY;
				face[0] = 0;
				face[1] = -stepY;
				face[2] = 0;
			}
			else
			{
				z += stepZ;
				tMaxZ += tDeltaZ;
				face[0] = 0;
				face[1] = 0;
				face[2] = -stepZ;
			}
		}
	}

	// there is no cube in range
	return nullptr;
}
