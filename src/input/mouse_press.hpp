#pragma once

#include <glm/vec2.hpp>

#include "input/key_mods.hpp"

namespace block_thingy::input {

struct mouse_press
{
	mouse_press(const glm::dvec2& position, int button, int action, int mods);
	mouse_press(const glm::dvec2& position, int button, int action, key_mods mods);

	glm::dvec2 position;
	int button;
	int action;
	key_mods mods;
};

}
