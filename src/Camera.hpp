#pragma once
// based on http://r3dux.org/2012/12/a-c-camera-class-for-simple-opengl-fps-controls/

#include <glm/vec3.hpp>

#include "fwd/Gfx.hpp"

namespace block_thingy {

class Camera
{
public:
	Camera(Gfx&);

	Camera(Camera&&) = delete;
	Camera(const Camera&) = delete;
	Camera& operator=(Camera&&) = delete;
	Camera& operator=(const Camera&) = delete;

	void mousemove(double mouseX, double mouseY, bool joystick = false);

	glm::dvec3 position;
	glm::dvec3 rotation;

private:
	double sensitivity;
	double joy_sensitivity;

	Gfx& gfx;
};

}
