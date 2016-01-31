#pragma once

#include <string>

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Phys.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "console/Console.hpp"
#include "gui/GUI.hpp"

struct GLFWwindow;

class Game
{
	public:
		explicit Game(GLFWwindow* window);
		virtual ~Game();

		void draw();

		void screenshot(const std::string& filename);

		static Game* instance;

		GLFWwindow* window;

		Camera cam;
		World world;
		Phys phys;
		Player player;
		Console console;
		GUI gui;

		static bool debug;

	private:
		double delta_time;
		FPSManager fps;
};