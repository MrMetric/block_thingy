#pragma once

#include <optional>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "fwd/World.hpp"
#include "fwd/physics/ray.hpp"
#include "fwd/physics/RaycastHit.hpp"
#include "types/window_size_t.hpp"

namespace block_thingy::physics {

// note: the origin is the near plane, not the camera
[[nodiscard]]
ray screen_pos_to_world_ray
(
	const glm::dvec2& mouse_pos,
	const window_size_t&,
	const glm::dmat4& view_matrix,
	const glm::dmat4& projection_matrix
);

[[nodiscard]]
std::optional<RaycastHit> raycast
(
	const World&,
	const ray&,
	double radius
);

}
