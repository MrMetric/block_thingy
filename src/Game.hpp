#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "fwd/Gfx.hpp"
#include "fwd/Player.hpp"
#include "World.hpp"
#include "block/BlockRegistry.hpp"
#include "fwd/block/BlockType.hpp"
#include "console/Command.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/EventManager.hpp"
#include "graphics/GUI/Base.hpp"
#include "physics/RaycastHit.hpp"
#include "position/ChunkInWorld.hpp"
#include "fwd/util/key_mods.hpp"
#include "util/Property.hpp"

#include "types/window_size_t.hpp"

#define LOGGER (Game::instance != nullptr \
	? Game::instance->console.logger \
	: std::cout \
)

class Game
{
	public:
		Game(Gfx&);

		Game(Game&&) = delete;
		Game(const Game&) = delete;
		void operator=(const Game&) = delete;

		void draw();
		void step_world();
		void draw_world();
		void open_gui(std::unique_ptr<Graphics::GUI::Base>);
		void quit();

		#ifdef USE_LIBPNG
		void screenshot(const std::string& filename);
		#endif
		void update_framebuffer_size(const window_size_t&);
		void keypress(int key, int scancode, int action, Util::key_mods);
		void charpress(char32_t, Util::key_mods);
		void mousepress(int button, int action, Util::key_mods);
		void mousemove(double x, double y);
		void joypress(int joystick, int button, bool pressed);
		void joymove(const glm::dvec2& motion);

		static Game* instance;

		BlockType block_type;
		std::unique_ptr<RaycastHit> hovered_block;

		// event_manager must be initialized before others!
		EventManager event_manager;
		Gfx& gfx;

		Camera camera;
		World world;
		Block::BlockRegistry block_registry;
		std::shared_ptr<Player> player_ptr;
		Player& player;
		Console console;
		KeybindManager keybinder;
		std::unique_ptr<Graphics::GUI::Base> gui;

		Property<bool> wireframe;

	private:
		double delta_time;
		FPSManager fps;

		Position::ChunkInWorld::value_type render_distance;

		void find_hovered_block(const glm::dmat4& projection_matrix, const glm::dmat4& view_matrix);

		std::vector<Command> commands;
		void add_commands();
};
