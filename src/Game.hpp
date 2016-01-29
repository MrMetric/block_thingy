#pragma once

#include "Camera.hpp"
#include "World.hpp"
#include "Phys.hpp"
#include "Player.hpp"
#include "gui/GUI.hpp"
#include "FPSManager.hpp"

class Game
{
	public:
		explicit Game(GLFWwindow* window);
		virtual ~Game();

		void draw();

		void screenshot(const std::string& filename);

		static Game* instance;

		World world;
		Camera cam;
		GLFWwindow* window;
		Phys phys;
		Player player;
		GUI gui;

		static bool debug;

	private:
		double delta_time;
		FPSManager fps;
};