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

		void update_framebuffer_size(int width, int height);
		void handleMouseMove(double mouseX, double mouseY);

		glm::dvec3 position;
		glm::dvec3 rotation;

	private:
		double sensitivity;

		GLFWwindow* window;
		glm::ivec2 window_mid;
};
