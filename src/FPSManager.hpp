#include <chrono>
#include <thread>

/** The FpsManager class is designed to work with GLFW and enforces a specified framerate on an application.
  * It can also display the current framerate at user-specified intervals, and in addition returns the time
  * duration since the last frame, which can be used to implement framerate independent movement.
  *
  * Author: r3dux
  * Revision: 0.3
  * Date: 1st September 2013
  *
  * ---- Creation examples (it's most useful to create your fpsManager object globally in your Main.cpp file): ----
  *
  *     FpsManager fpsManager(60.0);                    // Lock to 60fps, no reporting of framerate
  *
  *     FpsManager fpsManager(85.0, 3.0);               // Lock to 85fps, output FPS to console once every three seconds
  *
  *     FpsManager fpsManager(30.0, 0.5, "My App");     // Lock to 30fps, output FPS to console & window title every half second
  *
  *
  * ---- Using the fpsManager in your main loop: ----
  *
  * bool running     = true;
  * double deltaTime = 0.0;
  *
  * while (running)
  * {
  *     // Calculate our camera movement
  *     cam->move(deltaTime);
  *
  *     // Draw our scene
  *     drawScene();
  *
  *     // Exit if ESC was pressed or window was closed
  *     running = !glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED);
  *
  *     // Call our fpsManager to limit the FPS and get the frame duration to pass to the cam->move method
  *     deltaTime = fpsManager.enforceFPS();
  * }
  *
  * That's it! =D
  */

class FPSManager
{
	private:
		double frameStartTime;         // Frame start time
		double frameEndTime;           // Frame end time
		double frameDuration;          // How many milliseconds between the last frame and this frame

		double currentFps;             // The current FPS value
		double targetFrameDuration;    // How many milliseconds each frame should take to hit a target FPS value (i.e. 60fps = 1.0 / 60 = 0.016ms)

		double lastReportTime;         // The timestamp of when we last reported
		double reportInterval;         // How often to update the FPS value

		// FPSManager is padded with 4 bytes if this is a 32-bit int, so why not use all 64?
		uint64_t frameCount;        // How many frames have been drawn s

		// Private method to set relatively sane defaults. Called by constructors before overwriting with more specific values as required.
		void init(double theTargetFps)
		{
			setTargetFps(theTargetFps);

			frameCount     = 0;
			currentFps     = 0.0;
			frameStartTime = glfwGetTime();
			frameEndTime   = frameStartTime + 1;
			frameDuration  = 1;
			lastReportTime = frameStartTime;
			reportInterval = 1.0;
		}

	public:
		// Single parameter constructor - just set a desired framerate and let it go.
		// Note: No FPS reporting by default, although you can turn it on or off later with the setVerbose(true/false) method
		FPSManager(int theTargetFps)
		{
			init(theTargetFps);
		}

		void setTargetFps(double theFpsLimit)
		{
			targetFrameDuration = 1.0 / theFpsLimit;
		}

		// Returns the time it took to complete the last frame in milliseconds
		double getFrameDuration()
		{
			return frameDuration;
		}

		// Method to force our application to stick to a given frame rate and return how long it took to process a frame
		double enforceFPS()
		{
			// Get the current time
			frameEndTime = glfwGetTime();

			// Calculate how long it's been since the frameStartTime was set (at the end of this method)
			frameDuration = frameEndTime - frameStartTime;

			if(reportInterval != 0)
			{
				// Calculate and display the FPS every specified time interval
				if((frameEndTime - lastReportTime) > reportInterval)
				{
					// Update the last report time to be now
					lastReportTime = frameEndTime;

					// Calculate the FPS as the number of frames divided by the interval in seconds
					currentFps =  double(frameCount) / reportInterval;

					// Reset the frame counter to 1 (and not zero - which would make our FPS values off)
					frameCount = 1;
				}
				else // FPS calculation time interval hasn't elapsed yet? Simply increment the FPS frame counter
				{
					++frameCount;
				}
			} // End of if we specified a report interval section

			// Calculate how long we should sleep for to stick to our target frame rate
			double sleepDuration = targetFrameDuration - frameDuration;

			// If we're running faster than our target duration, sleep until we catch up!
			if(sleepDuration > 0.0)
			{
				std::chrono::duration<double> a(sleepDuration);
				std::this_thread::sleep_for(a);
			}

			// Reset the frame start time to be now - this means we only need put a single call into the main loop
			frameStartTime = glfwGetTime();

			// Pass back our total frame duration (including any sleep and the time it took to run this function) to be used as our deltaTime value
			return frameDuration + (frameStartTime - frameEndTime);

		}

		double getFPS()
		{
			return this->currentFps;
		}
};