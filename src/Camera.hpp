#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

namespace Position
{
	struct BlockInWorld;
}

class Camera
{
	public:
		void keypress(int key, int action);

		// Constructor
		Camera(GLFWwindow* window);

		// Mouse movement handler to look around
		void handleMouseMove(double mouseX, double mouseY);

		// Method to move the camera based on the current direction
		//void move(double delta_time);

		glm::dvec3 position;
		glm::dvec3 rotation;

	private:
		double pitchSensitivity;    // Controls how sensitive mouse movements affect looking up and down
		double yawSensitivity;      // Controls how sensitive mouse movements affect looking left and right

		GLFWwindow* window;
		int window_width;
		int window_height;
		int window_midX;
		int window_midY;

		// Holding any keys down?
		bool holdingForward;
		bool holdingBackward;
		bool holdingLeftStrafe;
		bool holdingRightStrafe;
};