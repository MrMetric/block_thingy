#include "Camera.hpp"
#include <glm/detail/func_trigonometric.hpp>	// glm::radians
#include <glm/detail/func_geometric.hpp>		// glm::normalize
#include "Coords.hpp"
#include "Game.hpp"

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

/*
// Function to calculate which direction we need to move the camera and by what amount
void Camera::move(double delta_time)
{
	// Vector to break up our movement into components along the X, Y and Z axis
	glm::dvec3 movement;

	// Get the sine and cosine of our X and Y axis rotation
	double sinXRot = sin( glm::radians( rotation.x ) );
	double cosXRot = cos( glm::radians( rotation.x ) );

	double sinYRot = sin( glm::radians( rotation.y ) );
	double cosYRot = cos( glm::radians( rotation.y ) );

	double pitchLimitFactor = cosXRot; // This cancels out moving on the Z axis when we're looking up or down

	if(holdingForward)
	{
		movement.x += sinYRot * pitchLimitFactor;
		movement.y += -sinXRot;
		movement.z += -cosYRot * pitchLimitFactor;
	}

	if(holdingBackward)
	{
		movement.x += -sinYRot * pitchLimitFactor;
		movement.y += sinXRot;
		movement.z += cosYRot * pitchLimitFactor;
	}

	if(holdingLeftStrafe)
	{
		movement.x += -cosYRot;
		movement.z += -sinYRot;
	}

	if(holdingRightStrafe)
	{
		movement.x += cosYRot;
		movement.z += sinYRot;
	}

	if(movement.x == 0 && movement.y == 0 && movement.z == 0)
	{
		return;
	}

	movement = glm::normalize(movement);

	// Calculate our value to keep the movement the same speed regardless of the framerate...
	double framerateIndependentFactor = movementSpeedFactor * delta_time;

	// .. and then apply it to our movement vector.
	movement *= framerateIndependentFactor;

	// Finally, apply the movement to our position
	position += movement;
}
*/

void Camera::keypress(int key, int action)
{
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch(key)
		{
			case 'W':
				this->holdingForward = true;
				break;
			case 'S':
				this->holdingBackward = true;
				break;
			case 'A':
				this->holdingLeftStrafe = true;
				break;
			case 'D':
				this->holdingRightStrafe = true;
				break;
			case 'Q':
				this->rotation.z -= 5;
				break;
			case 'E':
				this->rotation.z += 5;
				break;
		}
	}
	else
	{
		switch(key)
		{
			case 'W':
				this->holdingForward = false;
				break;
			case 'S':
				this->holdingBackward = false;
				break;
			case 'A':
				this->holdingLeftStrafe = false;
				break;
			case 'D':
				this->holdingRightStrafe = false;
				break;
		}
	}
}