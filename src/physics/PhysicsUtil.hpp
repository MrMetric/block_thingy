#pragma once

#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "fwd/World.hpp"
#include "fwd/physics/RaycastHit.hpp"
#include "types/window_size_t.hpp"

namespace block_thingy::physics {

void ScreenPosToWorldRay
(
	const glm::dvec2& mouse_pos,
	const window_size_t&,
	const glm::dmat4& view_matrix,
	const glm::dmat4& projection_matrix,
	glm::dvec3& out_origin,
	glm::dvec3& out_direction
);
std::unique_ptr<RaycastHit> raycast
(
	const World&,
	const glm::dvec3& origin,
	const glm::dvec3& direction,
	double radius
);

}
