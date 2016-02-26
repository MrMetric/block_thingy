#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/mat4x4.hpp>

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "console/Command.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/EventManager.hpp"
#include "gui/GUI.hpp"
#include "physics/RaytraceHit.hpp"

struct GLFWwindow;

class Game
{
	public:
		Game(GLFWwindow* window, int width, int height);

		void draw();
		#ifdef USE_LIBPNG
		void screenshot(const std::string& filename);
		#endif
		void update_framebuffer_size(int width, int height);
		void keypress(int key, int scancode, int action, int mods);
		void mousepress(int button, int action, int mods);
		void mousemove(double x, double y);

		static Game* instance;

		GLFWwindow* window;
		std::unique_ptr<RaytraceHit> hovered_block;

		Camera cam;
		Gfx gfx;
		Player player;
		World world;
		Console console;
		EventManager event_manager;
		GUI gui;

	private:
		double delta_time;
		FPSManager fps;

		void draw_world();
		void find_hovered_block(const glm::mat4& projection_matrix, const glm::mat4& view_matrix);

		std::vector<Command> commands;
		void add_commands();

		KeybindManager keybinder;
};
