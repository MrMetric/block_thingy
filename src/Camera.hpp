#pragma once

#include <glm/vec3.hpp>

struct GLFWwindow;

class Camera
{
	public:
		void keypress(int key, int action);

		explicit Camera(GLFWwindow* window);

		void handleMouseMove(double mouseX, double mouseY);

		glm::dvec3 position;
		glm::dvec3 rotation;

	private:
		double pitchSensitivity;	// Controls how sensitive mouse movements affect looking up and down
		double yawSensitivity;		// Controls how sensitive mouse movements affect looking left and right

		GLFWwindow* window;
		int window_width;
		int window_height;
		int window_midX;
		int window_midY;
};