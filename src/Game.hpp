#pragma once

#include <string>
#include <memory>

#include <glm/mat4x4.hpp>

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Phys.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "console/Console.hpp"
#include "gui/GUI.hpp"

struct GLFWwindow;
struct RaytraceHit;

class Game
{
	public:
		explicit Game(GLFWwindow* window);
		virtual ~Game();

		void draw();

		void screenshot(const std::string& filename);

		static Game* instance;
		static bool debug;

		GLFWwindow* window;
		std::unique_ptr<RaytraceHit> hovered_block;

		Camera cam;
		World world;
		Phys phys;
		Player player;
		Console console;
		GUI gui;

	private:
		double delta_time;
		FPSManager fps;

		void find_hovered_block(const glm::mat4& projection_matrix, const glm::mat4& view_matrix);
};