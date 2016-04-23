#pragma once
// based on http://r3dux.org/2012/12/a-c-camera-class-for-simple-opengl-fps-controls/

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct GLFWwindow;
class EventManager;

class Camera
{
	public:
		Camera(GLFWwindow* window, EventManager& event_manager);

		Camera(Camera&&) = delete;
		Camera(const Camera&) = delete;
		void operator=(const Camera&) = delete;

		void mousemove(double mouseX, double mouseY);

		glm::dvec3 position;
		glm::dvec3 rotation;

	private:
		double sensitivity;

		GLFWwindow* window;
		glm::dvec2 window_mid;
};
