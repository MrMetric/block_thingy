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
	rotation.x += (mouseY - window_midY) * sensitivity;
	rotation.y += (mouseX - window_midX) * sensitivity;

	if(rotation.x < -90) // limit looking up to vertically up
	{
		rotation.x = -90;
	}
	else if(rotation.x > 90) // limit looking down to vertically down
	{
		rotation.x = 90;
	}

	// Looking left and right - keep angles in the range 0 to 360
	// 0 degrees is looking directly down the negative Z axis "North", 90 degrees is "East", 180 degrees is "South", 270 degrees is "West"
	// We can also do this so that our 360 degrees goes -180 through +180 and it works the same, but it's probably best to keep our
	// range to 0 through 360 instead of -180 through +180.
	if(rotation.y < 0)
	{
		rotation.y += 360;
	}
	else if(rotation.y > 360)
	{
		rotation.y -= 360;
	}

	glfwSetCursorPos(window, window_midX, window_midY);
}
