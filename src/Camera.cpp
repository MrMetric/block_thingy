#include "Camera.hpp"

Camera::Camera(GLFWwindow* window)
	:
	pitchSensitivity(0.1),
	yawSensitivity(0.1),
	window(window),
	holdingForward(false),
	holdingBackward(false),
	holdingLeftStrafe(false),
	holdingRightStrafe(false)
{
	glfwGetWindowSize(window, &window_width, &window_height);

	window_midX = window_width >> 1;
	window_midY = window_height >> 1;

	glfwSetCursorPos(window, window_midX, window_midY);
}

// Function to deal with mouse position changes
void Camera::handleMouseMove(double mouseX, double mouseY)
{
	double horizMovement = (mouseX - this->window_midX+1) * this->yawSensitivity;
	double vertMovement  = (mouseY - this->window_midY) * this->pitchSensitivity;

	this->rotation.x += vertMovement;
	this->rotation.y += horizMovement;

	// Limit looking up to vertically up
	if(this->rotation.x < -90)
	{
		this->rotation.x = -90;
	}

	// Limit looking down to vertically down
	if(this->rotation.x > 90)
	{
		this->rotation.x = 90;
	}

	// Looking left and right - keep angles in the range 0.0 to 360.0
	// 0 degrees is looking directly down the negative Z axis "North", 90 degrees is "East", 180 degrees is "South", 270 degrees is "West"
	// We can also do this so that our 360 degrees goes -180 through +180 and it works the same, but it's probably best to keep our
	// range to 0 through 360 instead of -180 through +180.
	if(this->rotation.y < 0)
	{
		this->rotation.y += 360;
	}
	if(this->rotation.y > 360)
	{
		this->rotation.y -= 360;
	}

	glfwSetCursorPos(this->window, this->window_midX, this->window_midY);
}

void Camera::keypress(int key, int action)
{
	bool pressed = action == GLFW_PRESS || action == GLFW_REPEAT;

	switch(key)
	{
		case 'W':
			this->holdingForward = pressed;
			break;
		case 'S':
			this->holdingBackward = pressed;
			break;
		case 'A':
			this->holdingLeftStrafe = pressed;
			break;
		case 'D':
			this->holdingRightStrafe = pressed;
			break;
		case 'Q':
			if(pressed)
			{
				this->rotation.z -= 5;
			}
			break;
		case 'E':
			if(pressed)
			{
				this->rotation.z += 5;
			}
			break;
	}
}