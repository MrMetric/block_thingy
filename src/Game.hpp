#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Phys.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "console/Command.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "gui/GUI.hpp"

struct GLFWwindow;
struct RaytraceHit;

class Game
{
	public:
		explicit Game(GLFWwindow* window);

		void draw();
		void screenshot(const std::string& filename);
		void keypress(int key, int scancode, int action, int mods);

		static Game* instance;
		static bool debug;

		GLFWwindow* window;
		std::unique_ptr<RaytraceHit> hovered_block;

		Camera cam;
		Gfx gfx;
		Phys phys;
		Player player;
		World world;
		Console console;
		GUI gui;

	private:
		double delta_time;
		FPSManager fps;

		void find_hovered_block(const glm::mat4& projection_matrix, const glm::mat4& view_matrix);

		std::vector<Command> commands;
		void add_commands();

		KeybindManager keybinder;
};