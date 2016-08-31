#pragma once
// based on http://r3dux.org/2012/12/a-c-camera-class-for-simple-opengl-fps-controls/

#include <glm/vec3.hpp>

#include "fwd/Gfx.hpp"
#include "fwd/event/EventManager.hpp"

class Camera
{
	public:
		Camera(Gfx&, EventManager&);

		Camera(Camera&&) = delete;
		Camera(const Camera&) = delete;
		void operator=(const Camera&) = delete;

		void mousemove(double mouseX, double mouseY, bool joystick = false);

		glm::dvec3 position;
		glm::dvec3 rotation;

	private:
		double sensitivity;
		double joy_sensitivity;

		Gfx& gfx;
};
