#pragma once
// based on https://r3dux.org/2012/12/fpsmanager-a-c-helper-class-for-framerate-independent-movement/

#include <cassert>
#include <chrono>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace block_thingy {

class fps_manager
{
private:
	double frame_start_time;
	double fps;
	double min_frame_length;
	double last_update_time;
	double update_interval;

	// fps_manager is padded with 4 bytes if this is a 32-bit int, so why not use all 64?
	uint64_t frame_count;

public:
	fps_manager(const double max_fps)
	:
		frame_start_time(glfwGetTime()),
		fps(0),
		min_frame_length(1 / max_fps),
		last_update_time(frame_start_time),
		update_interval(1),
		frame_count(0)
	{
		assert(update_interval > 0);
	}

	double enforce_fps()
	{
		const double frame_end_time = glfwGetTime();
		const double frame_length = frame_end_time - frame_start_time;

		if(frame_end_time < last_update_time + update_interval)
		{
			++frame_count;
		}
		else
		{
			last_update_time = frame_end_time;
			fps = frame_count / update_interval;
			frame_count = 0;
		}

		if(frame_length < min_frame_length)
		{
			std::chrono::duration<double> a(min_frame_length - frame_length);
			std::this_thread::sleep_for(a);
		}

		frame_start_time = glfwGetTime();

		// the total frame length (including sleep and the time it took to run this function)
		// this is the delta_time value
		return frame_length + (frame_start_time - frame_end_time);
	}

	double get_fps() const
	{
		return fps;
	}
};

}
