#include "joy_press.hpp"

namespace block_thingy::input {

joy_press::joy_press(const int joystick, const int button, const int action)
:
	joystick(joystick),
	button(button),
	action(action)
{
}

}
