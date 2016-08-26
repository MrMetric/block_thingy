#include "Camera.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>

#include "Gfx.hpp"
#include "Util.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"

Camera::Camera(Gfx& gfx, EventManager& event_manager)
	:
	sensitivity(0.1),
	joy_sensitivity(1.2),
	gfx(gfx)
{
	event_manager.add_handler(EventType::window_size_change, [&gfx=gfx](const Event& event)
	{
		glfwSetCursorPos(gfx.window, gfx.window_mid.x, gfx.window_mid.y);
	});
}

void Camera::mousemove(const double mouseX, const double mouseY, bool joystick)
{
	double x = (mouseY - gfx.window_mid.y) * sensitivity;
	double y = (mouseX - gfx.window_mid.x) * sensitivity;
	if(joystick)
	{
		x *= joy_sensitivity;
		y *= joy_sensitivity;
	}
	rotation.x += x;
	rotation.y += y;

	// limit looking up/down to vertical
	rotation.x = Util::clamp(rotation.x, -90.0, 90.0);

	// keep left/right angle in range [0, 360)
	rotation.y = Util::mod(rotation.y, 360);

	glfwSetCursorPos(gfx.window, gfx.window_mid.x, gfx.window_mid.y);
}
