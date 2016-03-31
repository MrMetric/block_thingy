#include "Camera.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>

#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"

Camera::Camera(GLFWwindow* window, EventManager& event_manager)
	:
	sensitivity(0.1),
	window(window)
{
	event_manager.add_handler(EventType::window_size_change, [this, window](const Event& event)
	{
		auto e = static_cast<const Event_window_size_change&>(event);
		window_mid.x = e.width / 2;
		window_mid.y = e.height / 2;
		glfwSetCursorPos(window, window_mid.x, window_mid.y);
	});
}

void Camera::handleMouseMove(const double mouseX, const double mouseY)
{
	rotation.x += (mouseY - window_mid.y) * sensitivity;
	rotation.y += (mouseX - window_mid.x) * sensitivity;

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

	glfwSetCursorPos(window, window_mid.x, window_mid.y);
}
