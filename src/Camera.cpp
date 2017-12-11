#include "Camera.hpp"

#include <glm/common.hpp>

#include "Gfx.hpp"
#include "Util.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"

Camera::Camera(Gfx& gfx)
:
	sensitivity(0.1),
	joy_sensitivity(1.2),
	gfx(gfx)
{
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
	rotation.x = glm::clamp(rotation.x, -90.0, 90.0);

	// keep left/right angle in range [0, 360)
	rotation.y = glm::mod(rotation.y, 360.0);

	gfx.center_cursor();
}
