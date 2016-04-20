#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "World.hpp"
#include "console/Command.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/EventManager.hpp"
#include "gui/GUI.hpp"
#include "physics/RaycastHit.hpp"
#include "position/ChunkInWorld.hpp"
#include "util/Property.hpp"

class Player;

class Game
{
	public:
		Game(GLFWwindow* window, uint_fast32_t width, uint_fast32_t height);

		Game(Game&&) = delete;
		Game(const Game&) = delete;
		void operator=(const Game&) = delete;

		void draw();
		#ifdef USE_LIBPNG
		void screenshot(const std::string& filename);
		#endif
		void update_framebuffer_size(uint_fast32_t width, uint_fast32_t height);
		void keypress(int key, int scancode, int action, int mods);
		void mousepress(int button, int action, int mods);
		void mousemove(double x, double y);

		static Game* instance;

		GLFWwindow* window;
		std::unique_ptr<RaycastHit> hovered_block;

		// event_manager must be initialized before others!
		EventManager event_manager;

		Camera camera;
		Gfx gfx;
		World world;
		std::shared_ptr<Player> player_ptr;
		Player& player;
		Console console;
		GUI gui;

		Property<bool> wireframe;

	private:
		double delta_time;
		FPSManager fps;

		ChunkInWorld_type render_distance;

		void draw_world();
		void find_hovered_block(const glm::dmat4& projection_matrix, const glm::dmat4& view_matrix);

		std::vector<Command> commands;
		void add_commands();

		KeybindManager keybinder;
};
