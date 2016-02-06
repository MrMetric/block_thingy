#include "Camera.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>

Camera::Camera(GLFWwindow* window)
	:
	sensitivity(0.1),
	window(window)
{
	glfwGetWindowSize(window, &window_width, &window_height);

	window_midX = window_width / 2;
	window_midY = window_height / 2;

	glfwSetCursorPos(window, window_midX, window_midY);
}

void Camera::handleMouseMove(double mouseX, double mouseY)
{
	this->rotation.x += (mouseY - this->window_midY) * this->sensitivity;
	this->rotation.y += (mouseX - this->window_midX) * this->sensitivity;

	if(this->rotation.x < -90) // limit looking up to vertically up
	{
		this->rotation.x = -90;
	}
	else if(this->rotation.x > 90) // limit looking down to vertically down
	{
		this->rotation.x = 90;
	}

	// Looking left and right - keep angles in the range 0 to 360
	// 0 degrees is looking directly down the negative Z axis "North", 90 degrees is "East", 180 degrees is "South", 270 degrees is "West"
	// We can also do this so that our 360 degrees goes -180 through +180 and it works the same, but it's probably best to keep our
	// range to 0 through 360 instead of -180 through +180.
	if(this->rotation.y < 0)
	{
		this->rotation.y += 360;
	}
	else if(this->rotation.y > 360)
	{
		this->rotation.y -= 360;
	}

	glfwSetCursorPos(this->window, this->window_midX, this->window_midY);
}
