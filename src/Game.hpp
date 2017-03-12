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

#include "types/window_size_t.hpp"

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
		void draw_world(const glm::dvec3& position, const glm::dvec3& rotation);
		void open_gui(std::unique_ptr<Graphics::GUI::Base>);
		void quit();

		void screenshot(std::string filename);
		void update_framebuffer_size(const window_size_t&);
		void keypress(int key, int scancode, int action, Util::key_mods);
		void charpress(char32_t, Util::key_mods);
		void mousepress(int button, int action, Util::key_mods);
		void mousemove(double x, double y);
		void joypress(int joystick, int button, bool pressed);
		void joymove(const glm::dvec2& motion);

		template<typename T = Block::Base>
		BlockType add_block(const std::string& name)
		{
			BlockType t = block_registry.add<T>(name);
			add_block(name, t);
			return t;
		}

		static Game* instance;

		BlockType block_type;
		std::unique_ptr<RaycastHit> hovered_block;

		// event_manager must be initialized before others!
		EventManager event_manager;
		Gfx& gfx;

		Camera camera;
		Block::BlockRegistry block_registry; // must be initialized before world
		World world;
		std::shared_ptr<Player> player_ptr;
		Player& player;
		Console console;
		KeybindManager keybinder;
		std::unique_ptr<Graphics::GUI::Base> gui;

		bool wireframe;

	private:
		void add_block(const std::string& name, BlockType);

		double delta_time;
		FPSManager fps;

		Position::ChunkInWorld::value_type render_distance;

		void find_hovered_block();

		std::vector<Command> commands;
		void add_commands();
};
