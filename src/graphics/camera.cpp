#include "camera.hpp"

#include <glm/common.hpp>

#include "game.hpp"
#include "settings.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_change_setting.hpp"

namespace block_thingy::graphics {

camera::camera()
:
	mouse_sensitivity(settings::get<double>("mouse_sensitivity")),
	joystick_sensitivity(settings::get<double>("joystick_sensitivity"))
{
	game::instance->event_manager.add_handler(EventType::change_setting, [this](const Event& event)
	{
		const auto& e = static_cast<const Event_change_setting&>(event);

		if(e.name == "mouse_sensitivity")
		{
			mouse_sensitivity = *e.new_value.get<double>();
		}
		else if(e.name == "joystick_sensitivity")
		{
			joystick_sensitivity = *e.new_value.get<double>();
		}
	});
}

static void move(const glm::dvec2& angle, glm::dvec3& rotation)
{
	rotation.x += angle.y;
	rotation.y += angle.x;

	// limit looking up/down to vertical
	rotation.x = glm::clamp(rotation.x, -90.0, 90.0);

	// keep left/right angle in range [0, 360)
	rotation.y = glm::mod(rotation.y, 360.0);
}

void camera::mousemove(const glm::dvec2& offset)
{
	move(offset * mouse_sensitivity, rotation);
}

void camera::joymove(const glm::dvec2& offset)
{
	move(offset * joystick_sensitivity, rotation);
}

}
