#include "Camera.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>

#include "Util.hpp"
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

	// limit looking up/down to vertical
	rotation.x = Util::clamp(rotation.x, -90.0, 90.0);

	// keep left/right angle in range [0, 360)
	rotation.y = Util::mod(rotation.y, 360);

	glfwSetCursorPos(window, window_mid.x, window_mid.y);
}
