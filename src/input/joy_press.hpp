#pragma once

namespace block_thingy::input {

struct joy_press
{
	joy_press(int joystick, int button, int action);

	int joystick;
	int button;
	int action;
};

}
