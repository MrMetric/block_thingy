#pragma once

// based on http://r3dux.org/2012/12/a-c-camera-class-for-simple-opengl-fps-controls/

#include <glm/vec3.hpp>

struct GLFWwindow;

class Camera
{
	public:
		explicit Camera(GLFWwindow* window);

		void handleMouseMove(double mouseX, double mouseY);

		glm::dvec3 position;
		glm::dvec3 rotation;

	private:
		double sensitivity;

		GLFWwindow* window;
		int window_width;
		int window_height;
		int window_midX;
		int window_midY;
};
