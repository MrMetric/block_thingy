#pragma once

#include <optional>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "fwd/physics/ray.hpp"
#include "fwd/physics/raycast_hit.hpp"
#include "types/window_size_t.hpp"
#include "fwd/world/world.hpp"

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
std::optional<raycast_hit> raycast
(
	const world::world&,
	const ray&,
	double radius
);

}
