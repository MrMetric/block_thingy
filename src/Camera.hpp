#pragma once
// based on http://r3dux.org/2012/12/a-c-camera-class-for-simple-opengl-fps-controls/

#include <glm/vec3.hpp>

#include "fwd/Gfx.hpp"

namespace block_thingy {

class camera
{
public:
	camera(Gfx&);

	camera(camera&&) = delete;
	camera(const camera&) = delete;
	camera& operator=(camera&&) = delete;
	camera& operator=(const camera&) = delete;

	void mousemove(double mouseX, double mouseY, bool joystick = false);

	glm::dvec3 position;
	glm::dvec3 rotation;

private:
	double sensitivity;
	double joy_sensitivity;

	Gfx& gfx;
};

}
