#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace block_thingy {

class camera
{
public:
	camera();

	camera(camera&&) = delete;
	camera(const camera&) = delete;
	camera& operator=(camera&&) = delete;
	camera& operator=(const camera&) = delete;

	void mousemove(const glm::dvec2& offset);
	void joymove(const glm::dvec2& offset);

	glm::dvec3 position;
	glm::dvec3 rotation;

private:
	double mouse_sensitivity;
	double joystick_sensitivity;
};

}
