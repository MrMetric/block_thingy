#pragma once

#include <cstdint>
#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct RaytraceHit;
class World;

namespace PhysicsUtil
{
	void ScreenPosToWorldRay(
		const glm::dvec2& mouse_pos,
		const glm::uvec2& screen_size,
		const glm::dmat4& view_matrix,
		const glm::dmat4& projection_matrix,
		glm::dvec3& out_origin,
		glm::dvec3& out_direction
	);
	std::unique_ptr<RaytraceHit> raycast(const World& world, const glm::dvec3& origin, const glm::dvec3& direction, double radius);
};
